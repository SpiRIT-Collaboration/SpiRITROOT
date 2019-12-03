#include "STTransportModelEventGenerator.hh"
#include "FairLogger.h"
#include "FairRunSim.h"
#include "FairRootManager.h"
#include "FairMCEventHeader.h"
#include "FairIon.h"
#include "TSystem.h"
#include "TRandom.h"
#include "UrQMDParticle.hh"

STTransportModelEventGenerator::STTransportModelEventGenerator()
:FairGenerator(),
  fInputPath(), fInputName(), fInputFile(NULL), fInputTree(NULL),
  fB(-1.), 
  fBeamVector(NULL), fTargetVector(NULL), 
  fFillBeamVector(NULL), fFillTargetVector(NULL), 
  fPartArray(NULL),
  fCurrentEvent(0), fNEvents(0),
  fVertex(TVector3()), fVertexXYSigma(TVector2()), fTargetThickness(),
  fBeamAngle(TVector2()), fBeamAngleABSigma(TVector2()), fIsRandomRP(kTRUE)
{
}

STTransportModelEventGenerator::STTransportModelEventGenerator(TString fileName)
:FairGenerator("STTransportModelEvent",fileName),
  fInputPath(), fInputName(fileName), fInputFile(NULL), fInputTree(NULL),
  fB(-1.), fBeamVector(NULL), fTargetVector(NULL), 
  fFillBeamVector(NULL), fFillTargetVector(NULL), 
  fPartArray(NULL),
  fCurrentEvent(0), fNEvents(0),
  fVertex(TVector3()), fVertexXYSigma(TVector2(0.42,0.36)), fTargetThickness(0.083),
  fBeamAngle(TVector2()), fBeamAngleABSigma(TVector2()), fIsRandomRP(kTRUE)
{

  TString inputDir = gSystem->Getenv("VMCWORKDIR");
  fInputFile = new TFile(inputDir+"/input/"+fileName);

  RegisterFileIO();

}

STTransportModelEventGenerator::STTransportModelEventGenerator(TString filePath, TString fileName)
:FairGenerator("STTransportModelEvent",fileName),
  fInputPath(filePath), fInputName(fileName), fInputFile(NULL), fInputTree(NULL),
  fB(-1.), fBeamVector(NULL), fTargetVector(NULL), 
  fFillBeamVector(NULL), fFillTargetVector(NULL), 
  fPartArray(NULL),
  fCurrentEvent(0), fNEvents(0),
  fVertex(TVector3()), fVertexXYSigma(TVector2(0.42,0.36)), fTargetThickness(0.043),
  fBeamAngle(TVector2()), fBeamAngleABSigma(TVector2()), fIsRandomRP(kTRUE)
{

  fInputFile = new TFile(fInputPath+fileName);
  
  RegisterFileIO();

}

STTransportModelEventGenerator::~STTransportModelEventGenerator()
{
}

void STTransportModelEventGenerator::RegisterFileIO()
{
  if(!fInputFile->IsOpen())
    return;
  
  TString treeName, partBranchName;
  if(fInputName.BeginsWith("phits"))       { fGen = TransportModel::PHITS;  treeName = "tree";      partBranchName = "fparts"; }
  else if(fInputName.BeginsWith("amd"))    { fGen = TransportModel::AMD;    treeName = "amdTree";   partBranchName = "AMDParticle"; }
  else if(fInputName.BeginsWith("urqmd"))  { fGen = TransportModel::UrQMD;  treeName = "urqmdTree"; partBranchName = "partArray"; }
  else if(fInputName.BeginsWith("pBUU"))   { fGen = TransportModel::pBUU;   treeName = "tree";      partBranchName = ""; }
  else if(fInputName.BeginsWith("imqmd"))  { fGen = TransportModel::ImQMD;  treeName = "ImQMD";     partBranchName = "ImQMD"; }
  else
    LOG(FATAL)<<"STTransportModelEventGenerator cannot accept event files without specifying generator names."<<FairLogger::endl;
  
  fInputTree = (TTree*)fInputFile->Get(treeName);

  LOG(INFO)<<"-I Opening file: "<<fInputName<<FairLogger::endl;

  Bool_t isColSysFound = kFALSE;
  if(fInputTree->FindBranch("beamVector")&&fInputTree->FindBranch("targetVector")){
    isColSysFound = kTRUE;
    fInputTree -> SetBranchAddress("beamVector",&fBeamVector);
    fInputTree -> SetBranchAddress("targetVector",&fTargetVector);
  }
  if (fGen != TransportModel::pBUU) {
    fInputTree -> SetBranchAddress(partBranchName,&fPartArray);
    fInputTree -> SetBranchAddress("b",&fB);
  } else {
    fpBUU = new pBUUProcessor();
    fpBUU -> ConnectBranch(fInputTree);

    auto temp = TString(fInputFile -> GetName()).Tokenize("/");
    fB = TString(((TObjString *) temp -> At(5)) -> GetString()[1]).Atoi();
    delete temp;
  }

  fNEvents = fInputTree->GetEntries();

  fFillBeamVector   = new TClonesArray("TLorentzVector");
  fFillTargetVector = new TClonesArray("TLorentzVector");
  FairRootManager::Instance()->Register("BeamVector",   "Generator", fFillBeamVector,   isColSysFound);
  FairRootManager::Instance()->Register("TargetVector", "Generator", fFillTargetVector, isColSysFound);

}


Bool_t STTransportModelEventGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  if(!fInputTree||fCurrentEvent==fNEvents)
    return kFALSE;

  TVector3 eventVertex(0,0,0);  // gaus dist. in target XY, uniform in target Z
  eventVertex.SetX(gRandom->Gaus(fVertex.X(),fVertexXYSigma.X()));
  eventVertex.SetY(gRandom->Gaus(fVertex.Y(),fVertexXYSigma.Y()));
  eventVertex.SetZ(fVertex.Z()+gRandom->Uniform(-fTargetThickness*0.5,fTargetThickness*0.5));

  /** Event rotation **/
  // ex.) TVector3::RotateX() -> clockwise rotation in the direction of positive X-axis.
  // Generated event will be rotated w.r.t reaction plane at first.
  // To adapt the beam angle to the event, rotate ta w.r.t. Y axis at first.
  // Next, w.r.t the rotated X axis(X' axis), rotate tb'(like tb in rotated frame).

  // reaction plane.
  Double_t phiRP = 0.;
  if(fIsRandomRP)
    phiRP = gRandom->Uniform(-1,1)*TMath::Pi();

  // exchange ta, tb -> rotation angles
  Double_t beamAngleA = gRandom->Gaus(fBeamAngle.X(),fBeamAngleABSigma.X());
  Double_t beamAngleB = gRandom->Gaus(fBeamAngle.Y(),fBeamAngleABSigma.Y());
  Double_t tanBeamA  = TMath::Tan(beamAngleA);
  Double_t tanBeamB  = TMath::Tan(beamAngleB);

  TRotation rotatedFrame;             // set rotation operator for ta.
  rotatedFrame.RotateY(beamAngleA);   // rotate by angle-A of beam with respect to Y axis
  TVector3 axisX(1,0,0);              // define local X axis
  axisX.Transform(rotatedFrame);      // X' axis in rotated frame.
  Double_t thetaInRotatedFrame = -1.*TMath::ACos(TMath::Sqrt( (1.+tanBeamA*tanBeamA)/(tanBeamA*tanBeamA+tanBeamB*tanBeamB+1) ));  // tb' (be careful about the sign!!)
  TRotation rotateInRotatedFrame;
  rotateInRotatedFrame.Rotate(thetaInRotatedFrame,axisX); // rotate by angle-B' of beam with respect to X' axis.


  fInputTree -> GetEntry(fCurrentEvent);
  Int_t nPart = 0;
  if (fGen != TransportModel::pBUU)
    nPart = fPartArray->GetEntries();
  else
    nPart = fpBUU -> multi;
  auto event = (FairMCEventHeader*)primGen->GetEvent();
  if( event && !(event->IsSet()) ){
    event->SetEventID(fCurrentEvent);
    event->MarkSet(kTRUE);
    event->SetVertex(eventVertex);
    event->SetRotX(beamAngleA);
    event->SetRotY(beamAngleB);
    event->SetRotZ(phiRP);
    event->SetB(fB);
    event->SetNPrim(nPart);
  }

  fFillBeamVector->Clear();
  fFillTargetVector->Clear();
  if (fBeamVector)
    new((*fFillBeamVector)[0]) TLorentzVector(*fBeamVector);
  if (fTargetVector)
    new((*fFillTargetVector)[0]) TLorentzVector(*fTargetVector);

  Int_t numDiscarded = 0;
  for(Int_t iPart=0; iPart<nPart; iPart++){
    Int_t pdg;
    TVector3 p;
    TVector3 pos;

    switch(fGen){
      case TransportModel::PHITS:
	{
	  auto part = (PHITSParticle*)fPartArray->At(iPart);
	  pdg = kfToPDG(part->kf);
	  Double_t pMag = TMath::Sqrt(part->ke*(part->ke+2.*part->m))/1000.;
	  p = TVector3(part->mom[0],part->mom[1],part->mom[2]);
	  p.SetMag(pMag);
	  break;
	}
      case TransportModel::AMD:
	{
	  auto part = (AMDParticle*)fPartArray->At(iPart);
	  pdg = part->fPdg;
	  p = TVector3(part->fMomentum.Vect());
	  p.SetMag(p.Mag()/1000.);
	  break;
	}
      case TransportModel::UrQMD:
	{
	  auto part = (UrQMDParticle*)fPartArray->At(iPart);
	  pdg = part->GetPdg();
	  p = TVector3(part->GetMomentum().Vect());
	  p.SetMag(p.Mag()/1000.);
	  break;
	}
      case TransportModel::pBUU:
        {
          pdg = fpBUU -> GetPDG(iPart);
          if (pdg == 2112) {
            numDiscarded++;
            continue;
          }
          p = TVector3(fpBUU -> px[iPart], fpBUU -> py[iPart], fpBUU -> pz[iPart]);
          p.SetMag(p.Mag()/1000.);
          break;
        }
      case TransportModel::ImQMD:
        {
          auto part = (ImQMDParticle*)fPartArray->At(iPart);
          pdg = part->pdg;
          if (pdg == 2112) {
            numDiscarded++;
            continue;
          }
          p = TVector3(part->px, part->py, part->pz); // ImQMD model stores in GeV already
          break;
        }
      default:
	break;
    }
    auto Z = (pdg%10000000)/10000;
    if(Z > fMaxZ && fMaxZ > 0)
    {
      numDiscarded++;
      continue;
    }

    event -> SetNPrim(nPart - numDiscarded);

    p.SetPhi(p.Phi()+phiRP);  // random reaction plane orientation.
    p.RotateY(beamAngleA);    // rotate w.r.t Y axis
    p.Transform(rotateInRotatedFrame);

    // temporarily comment the code out because I don't know what it is doing
    //pos.SetPhi(pos.Phi()+phiRP);
    //pos.RotateY(beamAngleA);
    //pos.Transform(rotateInRotatedFrame);
    //pos += eventVertex;   // caution!! set the event to vertex position "after" the rotation operations!!

    primGen -> AddTrack(pdg, p.X(), p.Y(), p.Z(), eventVertex.X(), eventVertex.Y(), eventVertex.Z());
  }

  fCurrentEvent++;
  return kTRUE;
}

void STTransportModelEventGenerator::RegisterHeavyIon()
{

  TString symbol[50] = {"H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne",
    "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
    "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
    "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",
    "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn"};

  std::vector<Int_t> ions;
  for(Int_t i=0; i<fNEvents; i++){
    fInputTree->GetEntry(i);
    auto nPart = 0;
    if (fGen != TransportModel::pBUU)
      nPart = fPartArray -> GetEntries();
    else
      nPart = fpBUU -> multi;
    for(Int_t iPart=0; iPart<nPart; iPart++){
      Int_t pdg;
      switch(fGen){
	case TransportModel::PHITS:
	  {
	    auto part = (PHITSParticle*)fPartArray->At(iPart);
	    pdg = kfToPDG(part->kf);
	    break;
	  }
	case TransportModel::AMD:
	  {
	    auto part = (AMDParticle*)fPartArray->At(iPart);
	    pdg = part->fPdg;
	    break;
	  }
	case TransportModel::UrQMD:
	  {
	    auto part = (UrQMDParticle*)fPartArray->At(iPart);
	    pdg = part->GetPdg();
	    break;
	  }
       case TransportModel::pBUU:
         {
           pdg = fpBUU -> GetPDG(iPart);
           break;
         }
       case TransportModel::ImQMD:
         {
           auto part = (ImQMDParticle*) fPartArray->At(iPart);
           pdg = part->pdg;
           break;
         }
	default:
	  break;
      }
      if(pdg>3000)
	ions.push_back(pdg);
    }
  }
  std::sort(ions.begin(),ions.end());
  ions.resize(std::distance(ions.begin(),std::unique(ions.begin(),ions.end())));

  for(Int_t iIon=0; iIon<ions.size(); iIon++){
    auto z = (ions.at(iIon)%10000000)/10000;
    auto a = (ions.at(iIon)%10000)/10;

    if(z > 0) FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",a)+symbol[z-1],z,a,z));
  }


}


Int_t STTransportModelEventGenerator::kfToPDG(Long64_t kfCode)
{
  if (kfCode < 1000002)
    return kfCode;
  else {
    Int_t A = kfCode%1000000;
    Int_t Z = kfCode/1000000;

    return (1000000000 + Z*10000 + A*10);
  }
}

void STTransportModelEventGenerator::SetMaxZAllowed(int t_z)
{
  fMaxZ = t_z;
  LOG(INFO) << "Generator will discard all heavy ions with Z > " << fMaxZ << FairLogger::endl;
}

ClassImp(STTransportModelEventGenerator);
