#include "STSingleTrackInCMGenerator.hh"
#include "FairRunSim.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"
#include "TMath.h"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"
#include "TRandom.h"
#include "TFile.h"

BDCInfoLoader::BDCInfoLoader(int runNo, double vertexZ) : 
      fBeamFile(TString::Format("/mnt/spirit/rawdata/misc/Frozen_Information_For_SpiRIT_Analysis/Aug2019/BeamData/beam/beam_run%d.ridf.root", runNo)), 
      fVertexZ(vertexZ)
{
  fBeamTree = (TTree *) fBeamFile.Get("TBeam");
  fBeamTree -> SetBranchAddress("z", &fZ);
  fBeamTree -> SetBranchAddress("aoq", &fAoQ);
  fBeamTree -> SetBranchAddress("beta37", &fBeta37);
  fBDCTree = (TTree *) fBeamFile.Get("TBDC");
  fBDCTree -> SetBranchAddress("bdc1x", &fBDC1x);
  fBDCTree -> SetBranchAddress("bdc1y", &fBDC1y);
  fBDCTree -> SetBranchAddress("bdc2x", &fBDC2x);
  fBDCTree -> SetBranchAddress("bdc2y", &fBDC2y);
  fBDCTree -> SetBranchAddress("bdcax", &fBDCax);
  fBDCTree -> SetBranchAddress("bdcby", &fBDCby);

  fBeamEnergy = new STBeamEnergy();
  fBeamEnergy -> setBeam(runNo);
  fBDCProjection = new STBDCProjection(TString(gSystem -> Getenv("VMCWORKDIR")) + "/parameters/ReducedBMap.txt");
  fBDCProjection -> setBeam(runNo);
}

bool BDCInfoLoader::GetEntry(int i)
{
  if(fBeamTree -> GetEntry(i - 1) && fBDCTree -> GetEntry(i -1 ))
  {
    fBeamEnergy -> reset(fZ, fAoQ, fBeta37);
    Double_t E1 = fBeamEnergy -> getCorrectedEnergy();
    if (fZ > 0 && fZ < 75 && fAoQ > 1. &&
        fAoQ < 3 && fBDC1x > -999 && fBDC1y > -999 &&
        fBDC2x > -999 && fBDC2y > -999)
    {
      fBDCProjection -> ProjectParticle(fBDC2x, fBDC2y, -2160.,
                                        fBDCax, fBDCby, fZ,
                                        E1, -580.4 + fVertexZ, fBeamEnergy -> getMass());//-580.4,-583.904

      fProjA = fBDCProjection -> getA();
      fProjB = fBDCProjection -> getB();
    }
    else
    {
      fProjA = -999;
      fProjB = -999;
    }
    return true;
  }else return false;
}

MCBDCInfoLoader::MCBDCInfoLoader(const TString& fileName) : fBeamFile(fileName)
{
  fHeader = new STFairMCEventHeader;
  if(!fBeamFile.IsOpen())
    LOG(FATAL) << "Cannot load beam file from MC files " << fileName << FairLogger::endl; 
  fTree = (TTree*) fBeamFile.Get("cbmsim");
  fTree -> SetBranchAddress("MCEventHeader.", &fHeader);
  fTree -> SetBranchStatus("*", 0);
  fTree -> SetBranchStatus("MCEventHeader.*", 1);

  int n = fTree -> GetEntries();
  for(int i = 0; i < n; ++i)
  {
    fTree -> GetEntry(i);
    fEventIDToEntry[fHeader -> GetEventID()] = i;
  }
}

bool MCBDCInfoLoader::GetEntry(int i)
{
  if(fTree -> GetEntry(fEventIDToEntry[i]))
  {
    fProjA = 1000*fHeader -> GetRotX(); 
    fProjB = 1000*fHeader -> GetRotY();
  }
  else 
  {
    fProjA = -999;
    fProjB = -999;
  }
  return true;
}

ClassImp(STSingleTrackInCMGenerator);

STSingleTrackInCMGenerator::STSingleTrackInCMGenerator()
: FairGenerator("STSingleTrackInCMGenerator"),
  fNEvents(500)
{
  fPdgList.clear();
  RegisterHeavyIon();

}

STSingleTrackInCMGenerator::~STSingleTrackInCMGenerator()
{}

void STSingleTrackInCMGenerator::ReadConfig(const std::string& t_config)
{
  TrackParser parser(t_config);
  {
    // load from VMCWORKDIR/parameters
    auto vmc_dir = gSystem->Getenv("VMCWORKDIR");
    fVertexReader.OpenFile(std::string(vmc_dir) + "/parameters/" + parser.Get<std::string>("VertexFile"));
    SetNEvents(fVertexReader.GetNumEvent());
  }
  {
    auto bound = parser.GetBound("PtRange");
    fPtMin = bound.first;
    fPtMax = bound.second;
  }
  {
    auto bound = parser.GetBound("CMzRange");
    fCMzMin = bound.first;
    fCMzMax = bound.second;
  }

  for(int i = 0; i < 10; ++i)
    if(parser.KeyExist("PhiRange" + std::to_string(i)))
    {
      auto bound = parser.GetBound("PhiRange" + std::to_string(i));
      fPhiRanges.push_back({bound.first, bound.second});
    }

  fBeamMass = parser.Get<int>("BeamMass");
  fTargetMass = parser.Get<int>("TargetMass");
  fBeamEnergyPerN = parser.Get<double>("BeamEnergyPerN");

  fPdgList.push_back(parser.GetList<int>("Particle")[0]);
  if(fNoBeamRotation)
  {
    LOG(INFO)<<"Generator will not rotate event by beam angle." <<FairLogger::endl;
    fBDCInfo = std::unique_ptr<NullInfoLoader>(new NullInfoLoader);
  } else if(fMCBeamFile.IsNull())
  {
    LOG(INFO)<<"Loading BDC beam angle for run " << " " << fVertexReader.GetRunNo() <<FairLogger::endl;
    fBDCInfo = std::unique_ptr<BDCInfoLoader>(new BDCInfoLoader(fVertexReader.GetRunNo(), -13.2));
  } else {
    LOG(INFO)<<"Loading beam angle from: " << fMCBeamFile <<FairLogger::endl;
    fBDCInfo = std::unique_ptr<MCBDCInfoLoader>(new MCBDCInfoLoader(fMCBeamFile));
  }
}


Bool_t STSingleTrackInCMGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{

  if(fPdgList.size()<=0){
    LOG(INFO)<<"No initial track pdg is set !! -> only Proton is produced in this run."<<FairLogger::endl;
    fPdgList.push_back(2212);
  }

  Int_t pdg;  // pdg code of the particle for this event
  TVector3 momentum, vertex;
  if(fVertexReader.IsEnd())
  {
      LOG(ERROR) << "Number of events should equal to number of vertex. This should not happens";
      fVertexReader.LoopOver();
  }
  vertex = fVertexReader.GetVertex();
  fBDCInfo -> GetEntry(fVertexReader.GetEventID());
  fVertexReader.Next();

  pdg = fPdgList[0];

  // get phi range from random interval
  int phiInterval = (int) gRandom -> Uniform(0, fPhiRanges.size());
  double phi = gRandom -> Uniform(fPhiRanges[phiInterval].first, fPhiRanges[phiInterval].second);

  double pt = gRandom -> Uniform(fPtMin, fPtMax);
  double CMz = gRandom -> Uniform(fCMzMin, fCMzMax);
  
  TVector3 momentumCM(pt, 0, CMz);
  momentumCM.SetPhi(phi*TMath::DegToRad());

  // beam lorentzVector
  double EBeam = fBeamEnergyPerN*fBeamMass + fBeamMass*fNucleonMass;
  double PBeam = sqrt(EBeam*EBeam - fBeamMass*fBeamMass*fNucleonMass*fNucleonMass);
  TLorentzVector LV(0,0,PBeam,EBeam + fTargetMass*fNucleonMass);
 
  // transform CM vector to lab frame
  TLorentzVector pCM(momentumCM.x(), momentumCM.y(), momentumCM.z(), sqrt(momentumCM.Mag2() + GetA(pdg)*GetA(pdg)*fNucleonMass*fNucleonMass));
  pCM.Boost(LV.BoostVector());
  momentum.SetXYZ(pCM.Px(), pCM.Py(), pCM.Pz());

  // rotate according to beam angle
  Double_t proja = fBDCInfo -> GetProjA()/1000;
  Double_t projb = fBDCInfo -> GetProjB()/1000;
  Double_t tanBeamA  = TMath::Tan(proja);
  Double_t tanBeamB  = TMath::Tan(projb);

  TRotation rotatedFrame;             // set rotation operator for ta.
  rotatedFrame.RotateY(proja);   // rotate by angle-A of beam with respect to Y axis
  TVector3 axisX(1,0,0);              // define local X axis
  axisX.Transform(rotatedFrame);      // X' axis in rotated frame.
  Double_t thetaInRotatedFrame = -1.*TMath::ACos(TMath::Sqrt( (1.+tanBeamA*tanBeamA)/(tanBeamA*tanBeamA+tanBeamB*tanBeamB+1) ));  // tb' (be careful about the sign!!)
  TRotation rotateInRotatedFrame;
  rotateInRotatedFrame.Rotate(thetaInRotatedFrame,axisX); // rotate by angle-B' of beam with respect to X' axis.
  momentum.RotateY(proja);
  momentum.Transform(rotateInRotatedFrame);


  auto event = (FairMCEventHeader*)primGen->GetEvent();
  if( event && !(event->IsSet()) ){
    event->MarkSet(kTRUE);
    event->SetVertex(vertex);
  }

  primGen->AddTrack(pdg,momentum.X(),momentum.Y(),momentum.Z(),vertex.X(),vertex.Y(),vertex.Z());
  return kTRUE;
}

void STSingleTrackInCMGenerator::RegisterHeavyIon()
{

  TString symbol[50] = {"H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne",
    "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
    "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
    "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",
    "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn"};

  for(Int_t iH=2; iH<4; iH++)  // register d, t
    FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",iH)+symbol[0],1,iH,1));
  for(Int_t iHe=3; iHe<5; iHe++)       // register 3he, alpha
    FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",iHe)+symbol[1],2,iHe,2));
  FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",6)+symbol[1],2,6,2));
  for(Int_t iLi=6; iLi<8; iLi++)       // register 6Li, 7Li
    FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",iLi)+symbol[2],3,iLi,3));
  for(Int_t iBe=7; iBe<8; iBe++)       // register 7Be
    FairRunSim::Instance()->AddNewIon(new FairIon(Form("%d",iBe)+symbol[3],4,iBe,4));

}

Int_t STSingleTrackInCMGenerator::GetQ(Int_t pdg)
{
  TParticlePDG* part = TDatabasePDG::Instance()->GetParticle(pdg);
  if(part)
    return TMath::Abs(part->Charge()*1./3.);
  else if( (pdg%10000000)/10000<=120 && (pdg%10000000)/10000>-2 )
    return (pdg%10000000)/10000;
  else
    return 0;
}

Int_t STSingleTrackInCMGenerator::GetA(Int_t pdg)
{
  if(pdg==2212||pdg==2112)
    return 1;
  else if( (pdg%10000)/10<300 && (pdg%10000)/10>1 )
    return (pdg%10000)/10;
  else
    return 0;

}

