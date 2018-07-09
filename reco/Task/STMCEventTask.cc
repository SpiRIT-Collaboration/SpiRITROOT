#include "STMCEventTask.hh"
#include "FairRootManager.h"
#include "TSystem.h"
#include <fstream>
#include <string>
#include <sstream>
#include <vector>
#include "TRandom.h"
#include "STMCPoint.hh"
#include "STMCScintillatorHit.hh"

STMCEventTask::STMCEventTask()
  : FairTask("STMCEventTask"),
  fIsPersistence(kFALSE),
  fPrimaryTrackArray(NULL),
  fFairMCEventHeader(NULL),
  fTAMCPointArray(NULL),
  fMCEventHeader(NULL),
  fMCTriggerResponse(NULL),
  fScintillatorHitArray(NULL),
  fIsSetSystem(kFALSE),
  fBeamVectorArray(NULL),
  fTargetVectorArray(NULL)
{
}

STMCEventTask::~STMCEventTask()
{}

InitStatus STMCEventTask::Init()
{

  FairRootManager* ioman = FairRootManager::Instance();
  fPrimaryTrackArray = (TClonesArray*) ioman->GetObject("PrimaryTrack");
  fFairMCEventHeader = (FairMCEventHeader*) ioman->GetObject("MCEventHeader.");
  fTAMCPointArray    = (TClonesArray*) ioman->GetObject("TAMCPoint");
  if( ioman->GetBranchId("BeamVector")!=-1 && ioman->GetBranchId("TargetVector")!=-1 ){
    fIsSetSystem=kTRUE;
    fBeamVectorArray = (TClonesArray*) ioman->GetObject("BeamVector");
    fTargetVectorArray = (TClonesArray*) ioman->GetObject("TargetVector");
  }

  fMCEventHeader = new STMCEventHeader();
  fMCTriggerResponse = new STMCTriggerResponse();
  ioman -> Register("STMCEventHeader","ST",fMCEventHeader,fIsPersistence);
  ioman -> Register("STMCTriggerResponse","ST",fMCTriggerResponse,fIsPersistence);


  fScintillatorHitArray = new TClonesArray("STMCScintillatorHit");


  ReadCrosstalkFile();

  fkB = 0.07943; // mm/MeV
  fC  = 1e-5;
  fTopOfBar = -50.;  
  fAttLength = 2417.;  

  gRandom->SetSeed(0);

  return kSUCCESS;
}

void STMCEventTask::SetParContainers()
{}

void STMCEventTask::ReadCrosstalkFile()
{
  TString probFileDir = TString(gSystem->Getenv("VMCWORKDIR")) + "/parameters/";
  std::ifstream probFile(probFileDir+"kyotoCrosstalk.dat");
  std::string line, data;
  Char_t delimiter = ',';
  std::vector<Double_t> binArray, probArray;
  std::getline(probFile,line); // header
  for(Int_t i=0; i<64; i++)
    for(Int_t j=0; j<2; j++){
      binArray.clear();
      probArray.clear();
      std::getline(probFile,line);
      std::istringstream binStream(line);
      while(std::getline(binStream,data,delimiter))
	binArray.push_back(std::stod(data));
      std::getline(probFile,line);
      std::istringstream probStream(line);
      while(std::getline(probStream,data,delimiter))
	probArray.push_back(std::stod(data));
      fGraphProb[i][j] = new TGraph(binArray.size());
      fHistProb[i][j] = new TH1F(Form("fHistProb%d_%d",i,j),"",binArray.size()-1,0.5*(binArray.at(0)+binArray.at(1)),0.5*(binArray.at(binArray.size()-1)+binArray.back()));
      for(Int_t k=0; k<fGraphProb[i][j]->GetN(); k++){
	fGraphProb[i][j]->SetPoint(k,binArray.at(k),probArray.at(k));
	fHistProb[i][j]->Fill(binArray.at(k),probArray.at(k));
      }
    }

}

void STMCEventTask::Exec(Option_t* opt)
{
  fMCEventHeader->Clear();
  fScintillatorHitArray->Clear();

  fMCEventHeader->SetB(fFairMCEventHeader->GetB());
  TVector3 pv(fFairMCEventHeader->GetX(),fFairMCEventHeader->GetY(),fFairMCEventHeader->GetZ());
  pv.SetMag(pv.Mag()*10.);     // cm -> mm unit.
  fMCEventHeader->SetPrimaryVertex(pv);
  fMCEventHeader->SetBeamAngle(TVector2(fFairMCEventHeader->GetRotX(),fFairMCEventHeader->GetRotY()));
  fMCEventHeader->SetReactionPlane(fFairMCEventHeader->GetRotZ());
  fMCEventHeader->SetPrimaryTracks(fPrimaryTrackArray);
  if(fIsSetSystem){
    auto beam = (TLorentzVector*)fBeamVectorArray->At(0);
    fMCEventHeader->SetBeamVector(*beam);
    auto target = (TLorentzVector*)fTargetVectorArray->At(0);
    fMCEventHeader->SetTargetVector(*target);
  }
  std::vector<Int_t> detIDArray;  //
  for(Int_t iPoint=0; iPoint<fTAMCPointArray->GetEntries(); iPoint++){
    auto mcPoint = (STMCPoint*)fTAMCPointArray->At(iPoint);
    Int_t detID = mcPoint->GetDetectorID();
    if(detID==3)continue;

    auto itr = std::find(detIDArray.begin(), detIDArray.end(), detID);
    size_t index = std::distance(detIDArray.begin(), itr);
    STMCScintillatorHit* paddle = nullptr;

    if(index==detIDArray.size()){ // new detector ID
      Int_t nArray = fScintillatorHitArray->GetEntries();
      new((*fScintillatorHitArray)[nArray]) STMCScintillatorHit();
      paddle = (STMCScintillatorHit*)fScintillatorHitArray->At(nArray);
      paddle->SetDetectorID(detID);
      detIDArray.push_back(detID);
    }
    else paddle = (STMCScintillatorHit*) fScintillatorHitArray->At(index);

    Double_t dE = mcPoint->GetEnergyLoss()*1000.;  // GeV -> MeV
    if(detID>=24&&detID<=83){  // kyoto bar energy deposit    
      dE *= 1./(1.+fkB*dE+fC*dE*dE);                                   // birk's quenching formula
      dE *= TMath::Exp(-(fTopOfBar-mcPoint->GetY()*10.)/fAttLength);   // light attenuation
    }

    paddle->AddStep(dE, mcPoint);

  }


  std::vector<Int_t> kyotoIDArray;
  std::vector<Int_t> kyotoCTArray; // buffer for crosstalk candidate
  Int_t mult=0, multwCT=0;
  Double_t maxEdepKatana=0.;
  Int_t maxZKatana=0;
  for(Int_t iPaddle=0; iPaddle<fScintillatorHitArray->GetEntries(); iPaddle++){
    auto paddle = (STMCScintillatorHit*) fScintillatorHitArray->At(iPaddle);
    Int_t detID = paddle->GetDetectorID();
    Double_t dE = paddle->GetTotalEdep();
    Int_t maxZ  = paddle->GetMaxZ();

    if(detID>=84&&detID<=86){
      if(maxEdepKatana<=dE){
	maxEdepKatana = dE;
	maxZKatana = maxZ;
      }
    }

    if(detID>=24&&detID<=83&&dE>=1){

      mult++;

      Int_t EasirocChannel = GetChannelFromBar(detID);
      kyotoIDArray.push_back(EasirocChannel);
      Int_t CTChannelCand[2]={};
      GetPossibleVictim(EasirocChannel,CTChannelCand);
      Double_t CTProb[2]={};
      Double_t SaturationPoint[2];
      for(Int_t iCT=0; iCT<2; iCT++){
	Int_t saturateBin = fHistProb[EasirocChannel][iCT]->FindLastBinAbove(0.5);
	SaturationPoint[iCT] = fHistProb[EasirocChannel][iCT]->GetBinCenter(saturateBin);
	if( dE<=SaturationPoint[iCT] )
	  if( fGraphProb[EasirocChannel][iCT]->Eval(dE) > gRandom->Uniform(0,1) )
	    kyotoCTArray.push_back(CTChannelCand[iCT]);
	if( dE>SaturationPoint[iCT] )
	  if( fHistProb[EasirocChannel][iCT]->Integral(saturateBin-10,saturateBin)/10. > gRandom->Uniform(0,1) )
	    kyotoCTArray.push_back(CTChannelCand[iCT]);
      }

    }
  }

  multwCT = mult;
  std::sort(kyotoCTArray.begin(),kyotoCTArray.end());
  kyotoCTArray.erase( std::unique(kyotoCTArray.begin(), kyotoCTArray.end()), kyotoCTArray.end());
  for(auto CTChannel: kyotoCTArray){
    auto itr = std::find(kyotoIDArray.begin(), kyotoIDArray.end(), CTChannel);
    size_t index = std::distance( kyotoIDArray.begin(), itr);
    if( index==kyotoIDArray.size() ) // there are no hit in current index -> register as a crosstalk
      multwCT++;
  }

  Bool_t isTrigger = kFALSE;
  if(multwCT>=4&&maxEdepKatana<=150)
    isTrigger = kTRUE;

  fMCTriggerResponse->SetIsTrigger(isTrigger);
  fMCTriggerResponse->SetKyotoMult(mult);
  fMCTriggerResponse->SetKyotoMultWCT(multwCT);
  fMCTriggerResponse->SetKatanaVetoMaxEdep(maxEdepKatana);
  fMCTriggerResponse->SetKatanaVetoMaxZ(maxZKatana);
  fMCTriggerResponse->SetScintillatorHitArray(fScintillatorHitArray);


}

Int_t STMCEventTask::GetChannelFromBar(Int_t detID)
{
  // id assignment in geomSpiRIT
  // 
  // DS 59 57  ...  3 1 US
  //    =============== : beam right
  //                      <- beam direction
  //    =============== : beam left
  //    58 56  ...  2 0
  //
  //
  // channel assignment in EASIROC
  //
  // 32 34 ... 60 33 35 ... 61
  // ========================= : beam right
  //
  // ========================= : beam left
  // 0  2  ... 28 1  3  ... 29
  //
  // (30,31,62,63 are unused.)
  //

  Int_t barID = detID-24;
  if(barID<0||barID>59) return -1; // this may kill the excution.
  if(barID%2==0){
    barID /= 2;
    barID = 29-barID;
    if(barID>=0&&barID<=14) barID *= 2;
    else if(barID>=15&&barID<=29){
      barID = barID-15;
      barID = 2*barID+1;
    }
  }
  else if(barID%2!=0){
    barID = (barID-1)/2;
    barID = 29-barID;
    if(barID>=0&&barID<=14) barID = barID*2+32;
    else if(barID>=15&&barID<=29){
      barID = barID-15;
      barID = 2*barID+33;
    }
  }

  return barID;

}

void STMCEventTask::GetPossibleVictim(Int_t source, Int_t *victim)

{
  if( source==0 || source==1 || source==32 || source==33 )
    victim[0] = source+2;
  else if( source==30 || source==31 || source==62 || source==63 )
    victim[0] =source-2;
  else if( (source>=2&&source<=7) || (source>=12&&source<=21) || 
      (source>=26&&source<=29) || (source>=34&&source<=39) || 
      (source>=44&&source<=53) || (source>=58&&source<=61) ){
    victim[0] = source-2;
    victim[1] = source+2;
  }
  else if( source==8 || source==9 || source==40 || source==41 ){
    victim[0] = source-2;
    victim[1] = source+16;
  }
  else if( source==10 || source==11 || source==42 || source==43 ){
    victim[0] = source+2;
    victim[1] = source+12;
  }
  else if(source==22){
    victim[0] = 11;
    victim[1] = 20;
  }
  else if(source==23){
    victim[0] = 10;
    victim[1] = 21;
  }
  else if( source==24 || source==25 || source==56 || source==57 ){
    victim[0] = source-16;
    victim[1] = source+2;
  }
  else if( source==54 || source==55 ){
    victim[0] = source-12;
    victim[1] = source-2;
  }

}


void STMCEventTask::SetPersistence(Bool_t value) { fIsPersistence = value; }


ClassImp(STMCEventTask);
