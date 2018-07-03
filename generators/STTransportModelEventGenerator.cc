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
  fInputFile(NULL), fInputTree(NULL),
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
  fInputFile(NULL), fInputTree(NULL),
  fB(-1.), fBeamVector(NULL), fTargetVector(NULL), 
  fFillBeamVector(NULL), fFillTargetVector(NULL), 
  fPartArray(NULL),
  fCurrentEvent(0), fNEvents(0),
  fVertex(TVector3()), fVertexXYSigma(TVector2(0.42,0.36)), fTargetThickness(0.083),
  fBeamAngle(TVector2(-0.06,0.)), fBeamAngleABSigma(TVector2()), fIsRandomRP(kTRUE)
{
  TString treeName, partBranchName;
  if(fileName.BeginsWith("phits"))       { fGen = TransportModel::PHITS;  treeName = "tree";      partBranchName = "fparts"; }
  else if(fileName.BeginsWith("amd"))    { fGen = TransportModel::AMD;    treeName = "amdTree";   partBranchName = "AMDParticle"; }
  else if(fileName.BeginsWith("urqmd"))  { fGen = TransportModel::UrQMD;  treeName = "urqmdTree"; partBranchName = "partArray"; }
  else
    LOG(FATAL)<<"STTransportModelEventGenerator cannot accept event files without specifying generator names."<<FairLogger::endl;

  TString inputDir = gSystem->Getenv("VMCWORKDIR");
  fInputFile = new TFile(inputDir+"/input/"+fileName);
  fInputTree = (TTree*)fInputFile->Get(treeName);

  LOG(INFO)<<"-I Opening file: "<<fileName<<FairLogger::endl;

  fInputTree -> SetBranchAddress("b",&fB);
  fInputTree -> SetBranchAddress("beamVector",&fBeamVector);
  fInputTree -> SetBranchAddress("targetVector",&fTargetVector);
  fInputTree -> SetBranchAddress(partBranchName,&fPartArray);

  fNEvents = fInputTree->GetEntries();


  fFillBeamVector   = new TClonesArray("TLorentzVector");
  fFillTargetVector = new TClonesArray("TLorentzVector");
  FairRootManager::Instance()->Register("BeamVector", "Generator", fFillBeamVector, kTRUE);
  FairRootManager::Instance()->Register("TargetVector", "Generator", fFillTargetVector, kTRUE);
}

STTransportModelEventGenerator::~STTransportModelEventGenerator()
{
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
  Int_t nPart = fPartArray->GetEntries();
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
  new((*fFillBeamVector)[0]) TLorentzVector(*fBeamVector);
  new((*fFillTargetVector)[0]) TLorentzVector(*fTargetVector);

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
      default:
	break;
    }

    p.SetPhi(p.Phi()+phiRP);  // random reaction plane orientation.
    p.RotateY(beamAngleA);    // rotate w.r.t Y axis
    p.Transform(rotateInRotatedFrame);

    pos.SetPhi(pos.Phi()+phiRP);
    pos.RotateY(beamAngleA);
    pos.Transform(rotateInRotatedFrame);
    pos += eventVertex;   // caution!! set the event to vertex position "after" the rotation operations!!

    primGen -> AddTrack(pdg, p.X(), p.Y(), p.Z(), pos.X(), pos.Y(), pos.Z());
  }

  fCurrentEvent++;
  return kTRUE;
}

void STTransportModelEventGenerator::RegisterHeavyIon()
{
  if(!fInputFile)
    return;

  TString symbol[50] = {"H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne",
    "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
    "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
    "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",
    "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn"};

  std::vector<Int_t> ions;
  for(Int_t i=0; i<fNEvents; i++){
    fInputTree->GetEntry(i);
    for(Int_t iPart=0; iPart<fPartArray->GetEntries(); iPart++){
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

    FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",a)+symbol[z-1],z,a,z));
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

ClassImp(STTransportModelEventGenerator);
