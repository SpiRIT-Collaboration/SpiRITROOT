#include "STMCEventTask.hh"
#include "FairRootManager.h"
#include "TSystem.h"
#include <fstream>
#include <string>
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
  fBeamA(132), fBeamZ(50), fBeamE(270.),
  fTargetA(124), fTargetZ(50),
  fNProbBin(0),
  fProbGraph(nullptr)
{}

STMCEventTask::~STMCEventTask()
{}

InitStatus STMCEventTask::Init()
{

   FairRootManager* ioman = FairRootManager::Instance();
   fPrimaryTrackArray = (TClonesArray*) ioman->GetObject("PrimaryTrack");
   fFairMCEventHeader = (FairMCEventHeader*) ioman->GetObject("MCEventHeader.");
   fTAMCPointArray    = (TClonesArray*) ioman->GetObject("TAMCPoint");

   fMCEventHeader = new STMCEventHeader();
   fMCTriggerResponse = new STMCTriggerResponse();
   ioman -> Register("STMCEventHeader","ST",fMCEventHeader,fIsPersistence);
   ioman -> Register("STMCTriggerResponse","ST",fMCTriggerResponse,fIsPersistence);

   fScintillatorHitArray = new TClonesArray("STMCScintillatorHit");

   fMCEventHeader -> SetBeamA(fBeamA);
   fMCEventHeader -> SetBeamZ(fBeamZ);
   fMCEventHeader -> SetBeamE(fBeamE);
   fMCEventHeader -> SetTargetA(fTargetA);
   fMCEventHeader -> SetTargetZ(fTargetZ);


   TString probFileName = TString(gSystem->Getenv("VMCWORKDIR")) + "/parameters/" + fCTFileName;
   std::ifstream probFile;
  /* 
   while(!probFile.eof())
      probFile <<  <<  ;

   }*/
   // temporary
   std::ifstream inFile;
   Int_t bbuf[2][50]; // bin number buffer
   //Double_t pbuf[2][50]; // prob. buffer
   Double_t abuf[2][50]; // ADC buffer
   TString fpath = TString(gSystem->Getenv("VMCWORKDIR"))+"/input/";
   //TString fpath = "/cache/scr/spirit/kaneko/ana/kyoto/";
   for(Int_t i=0; i<2; i++){
      inFile.open( Form(fpath+"probabilitylist%d.txt",i),std::ios::in);
      std::string line;
      Int_t j=0;
      while(std::getline(inFile,line)){
	 sscanf(line.data(),"%d %lf %lf", &bbuf[i][j], &pbuf[i][j], &abuf[i][j]);
	 //std::cout << pbuf[i][j] << std::endl;
	 j++;
      }
   }

   probFile.close();

   gRandom->SetSeed(0);

   return kSUCCESS;
}

void STMCEventTask::SetParContainers()
{}

void STMCEventTask::Exec(Option_t* opt)
{
   fMCEventHeader->Clear();
   fScintillatorHitArray->Clear();

   fMCEventHeader->SetB(fFairMCEventHeader->GetB());
   TVector3 pv(fFairMCEventHeader->GetX(),fFairMCEventHeader->GetY(),fFairMCEventHeader->GetZ());
   pv.SetMag(pv.Mag()*10.);	// cm -> mm unit.
   fMCEventHeader->SetPrimaryVertex(pv);
   fMCEventHeader->SetBeamAngle(TVector2(fFairMCEventHeader->GetRotX(),fFairMCEventHeader->GetRotY()));
   fMCEventHeader->SetReactionPlane(fFairMCEventHeader->GetRotZ());
   fMCEventHeader->SetPrimaryTracks(fPrimaryTrackArray);

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

      Double_t de = mcPoint->GetEnergyLoss()*1000.;  // GeV -> MeV
      if(detID>=24&&detID<=83)	// attenuation in kyoto.
	 de *= TMath::Exp(-(-5.-mcPoint->GetY())/241.7);

      paddle->AddStep(de, mcPoint);

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

      if(detID>=24&&detID<=83&&dE>=0.5){
	    
	 mult++;
         
	 Int_t barID = detID-24;
	 if(barID%2==0){
	    barID=barID/2;
	    barID=29-barID;
	    if(barID>=15) barID++;
	 }else{
	    barID = (barID-1)/2 +30;
	    barID=59+30-barID;
	    barID+=2;
	    if(barID>=47)barID++;
	 }
	 kyotoIDArray.push_back(barID);

         // temporary thing
	 // make crosstalk
	 for(Int_t ib=0; ib<50; ib++){
	    if(ib<=dE&&ib+1>dE){
	       if(gRandom->Uniform(0,1)<pbuf[0][ib]&&(barID!=0&&barID!=16&&barID!=32&&barID!=48))
		  kyotoCTArray.push_back(barID-1);
	       if(gRandom->Uniform(0,1)<pbuf[1][ib])
		  kyotoCTArray.push_back(barID+1);
	    }else if(dE>50){ // high dE: (saturation) -> use constant probability
	       if(gRandom->Uniform(0,1)<0.65&&(barID!=0&&barID!=32&&barID!=16&&barID!=48))
		  kyotoCTArray.push_back(barID-1);
	       if(gRandom->Uniform(0,1)<0.7)
		  kyotoCTArray.push_back(barID+1);
	    }
	 }
      }
   }

   multwCT = mult;
   std::sort(kyotoCTArray.begin(),kyotoCTArray.end());
   kyotoCTArray.erase( std::unique(kyotoCTArray.begin(), kyotoCTArray.end()), kyotoCTArray.end());
   for(Int_t iCT=0; iCT<kyotoCTArray.size(); iCT++){
      auto itr = std::find(kyotoIDArray.begin(), kyotoIDArray.end(), kyotoCTArray.at(iCT));
      size_t index = std::distance( kyotoIDArray.begin(), itr);
      if( index==kyotoIDArray.size() )
	 multwCT++;
   }
   
   Bool_t isTrigger = kFALSE;
   if(multwCT>=4&&maxEdepKatana<=150)
      isTrigger = kTRUE;

   fMCTriggerResponse->SetIsTrigger(isTrigger);
   fMCTriggerResponse->SetMult(mult);
   fMCTriggerResponse->SetMultWCT(multwCT);
   fMCTriggerResponse->SetVetoMaxEdep(maxEdepKatana);
   fMCTriggerResponse->SetVetoMaxZ(maxZKatana);
   fMCTriggerResponse->SetScintillatorHitArray(fScintillatorHitArray);


}

void STMCEventTask::SetPersistence(Bool_t value) { fIsPersistence = value; }

ClassImp(STMCEventTask);
