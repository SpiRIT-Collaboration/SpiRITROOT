#include "STModelToLabFrameGenerator.hh"
#include "FairLogger.h"
#include "FairRunSim.h"
#include "FairRootManager.h"
#include "FairMCEventHeader.h"
#include "FairIon.h"
#include "TSystem.h"
#include "TRandom.h"

#include "STFairMCEventHeader.hh"
#include "TDatabasePDG.h"
#include "TParticlePDG.h"

TParticlePDG* Elements::PDGToParticleData(int pdg)
{
  if(auto particle = TDatabasePDG::Instance()->GetParticle(pdg))
    return particle;
  else
  {
    // heavy ioins. Otherwise TDatabasePDG should already have the information
    int Z = (pdg%10000000)/10000;
    int A = (pdg%10000)/10;
    // add particle to TDatabasePDG so the memory management behavior is identical for light particles or ions.
    return TDatabasePDG::Instance()->AddParticle(symbol[Z], symbol[Z], A*Elements::kProtonMass, true, 0, 3*Z, "ion", pdg, 0, 0); 
  }
}

/***********************************************
 * Transport Reader base class
 * *********************************************/
STTransportReader::STTransportReader()
{}

STTransportReader::~STTransportReader()    
{}

std::vector<FairIon*> STTransportReader::GetParticleList()
{
  int initEventID = this -> GetEntry();

  auto n = this -> GetEntries();
  std::set<int> pdgList;
  std::vector<STTransportParticle> particles;
  while(this -> GetNext(particles))
  {
    for(const auto& part : particles) 
      if(part.pdg > 3000) // if pdg <= 3000 it is consider a regular elements, not heavy ions
        pdgList.insert(part.pdg);
  }

  std::vector<FairIon*> particleList;
  for(auto pdg : pdgList)
  {
    auto particle = Elements::PDGToParticleData(pdg);
    int a = int(particle -> Mass()/Elements::kProtonMass + 0.5);
    int z = int(particle -> Charge()/3. + 0.5);
    if(a > 1) particleList.push_back(new FairIon(Form("%d",a)+Elements::symbol[z-1],z,a,z));
  }

  // rewind the reader to its initial point
  this -> SetEntry(initEventID);
  return particleList; 
}

/**********************************************
 * Deried reader class for ImQMD particles
 * ********************************************/
STImQMDReader::STImQMDReader(TString fileName) : fFile(fileName), fLocalID(0)
{
  if(!fFile.IsOpen())
    LOG(INFO) << "Reading ImQMD data from " << fileName << FairLogger::endl;

  fTree = (TTree*) fFile.Get("cluster");
  if(!fTree)
    LOG(FATAL) << "Tree cluster is not found in file " << fileName << FairLogger::endl;
  
  fTree -> SetBranchAddress("eventid", &fTreeEventID);
  fTree -> SetBranchAddress("A", &fPartA);
  fTree -> SetBranchAddress("Z", &fPartZ);
  fTree -> SetBranchAddress("px", &fPx);
  fTree -> SetBranchAddress("py", &fPy);
  fTree -> SetBranchAddress("pz", &fPz);
  fTree -> SetBranchAddress("x", &fX);
  fTree -> SetBranchAddress("y", &fY);
  fTree -> SetBranchAddress("z", &fZ);
  int n = fTree -> GetEntries();
  fTree -> GetEntry(n - 1);
  fEntries = fTreeEventID;

  fTree -> GetEntry(fLocalID);
}

TString STImQMDReader::Print()
{ return TString::Format("ImQMD reader with source %s", fFile.GetName()); }

STImQMDReader::~STImQMDReader() {}

bool STImQMDReader::GetNext(std::vector<STTransportParticle>& particleList)
{
  particleList.clear();
  if(fEventID >= fEntries) return false;
  while(fTreeEventID == fEventID)
  {
    int pdg = 0;
    int Z = fPartZ, A = fPartA;
    if(Z == 1 && A == 1) pdg = 2212;
    else if(Z == 0 && A == 1) pdg = 2122;
    // special case for ImQMD files
    // When Z = +/- 1 and A = 0, it refers to pions
    else if(Z == 1 && A == 0) pdg = 211;
    else if(Z == -1 && A == 0) pdg = -211;
    else pdg = 1000000000 + Z*10000 + A*10;

    particleList.push_back({pdg, fPx, fPy, fPz, fX, fY, fZ});
    ++fLocalID;
    fTree -> GetEntry(fLocalID);
  }
  ++fEventID;
  return true;;
}

void STImQMDReader::SetEntry(int t_entry)
{
  fLocalID = 0;
  fTree -> GetEntry(fLocalID);
  while(fTreeEventID < t_entry)
  {
    ++fLocalID;
    if(fLocalID > fTree->GetEntries()) 
      LOG(FATAL) << "ImQMD Reader cannot find start entry for event " << t_entry << FairLogger::endl;
    fTree -> GetEntry(fLocalID);
  }
  fEventID = t_entry;
}

/*************************************************
* STpBUUReader
**************************************************/
STpBUUReader::STpBUUReader(TString fileName) : fFile(fileName)
{
  if(fFile.IsOpen())
    LOG(INFO) << "Reading pBUU data from " << fileName << FairLogger::endl;
  fTree = (TTree*) fFile.Get("tree");
  if(!fTree)
    LOG(FATAL) << "Tree is not found in pBUU file " << fileName << "!" << FairLogger::endl;

  fTree -> SetBranchAddress("multi", &fMulti);
  fTree -> SetBranchAddress("pid", fPID);
  fTree -> SetBranchAddress("px", fPx);
  fTree -> SetBranchAddress("py", fPy);
  fTree -> SetBranchAddress("pzCMS", fPz);
  fTree -> SetBranchAddress("x", fX);
  fTree -> SetBranchAddress("y", fY);
  fTree -> SetBranchAddress("z", fZ);
  fTree -> SetBranchAddress("multi", &fMulti);

  fEntries = fTree -> GetEntries();
}

bool STpBUUReader::GetNext(std::vector<STTransportParticle>& particleList)
{
  particleList.clear();
  if(fEventID < fEntries)
  {
    fTree -> GetEntry(fEventID);
    int pdg;
    for(int i = 0; i < fMulti; ++i)
    {
      switch(fPID[i])
      {
        case 1: pdg = 2212; break;
        case 2: pdg = 2122; break;
        case 3: pdg = 1000010020; break;
        case 4: pdg = 1000020030; break;
        case 5: pdg = 1000010030; break;
        case 6: pdg = 1000020040; break;
        default: continue;
      }
      particleList.push_back({pdg, (double) fPx[i]/1000., (double) fPy[i]/1000., (double) fPz[i]/1000., // convert to GeV 
                                   (double) fX[i], (double) fY[i], (double) fZ[i]});
    }
    ++fEventID;
    return true;
  }
  else return false;
}

TString STpBUUReader::Print()
{ return TString::Format("pBUU reader with source %s", fFile.GetName()); }

std::vector<FairIon*> STpBUUReader::GetParticleList()
{
  std::vector<FairIon*> particleList;
  // pBUU only support ions of up to He4
  for(auto pdg : std::vector<std::pair<int, int>>{{2,1},{3,1},{3,2},{4,2}})
  {
    int a = pdg.first, z = pdg.second;
    particleList.push_back(new FairIon(Form("%d",a)+Elements::symbol[z-1],z,a,z));
  }
  return particleList; 
}


/**************************************************
 * Event generator itself
 * ************************************************/

STModelToLabFrameGenerator::STModelToLabFrameGenerator()
:FairGenerator(),
  fReader(nullptr), fInputPath(), fInputName(), fB(-1.), 
  fBeamInfo(NULL), fHeavyIonsRegistered(kFALSE),
  fCurrentEvent(0), fNEvents(0),
  fVertex(TVector3()), fVertexXYSigma(TVector2()), fTargetThickness(),
  fBeamAngle(TVector2(-0.022,0.)), fBeamAngleABSigma(TVector2(0.035,0)), fIsRandomRP(kTRUE)
{
  this -> RegisterReader();
}

STModelToLabFrameGenerator::STModelToLabFrameGenerator(TString fileName)
:FairGenerator("STModelToLabFrame",fileName),
  fReader(nullptr), fInputPath(), fInputName(fileName),
  fB(-1.), 
  fBeamInfo(NULL), fHeavyIonsRegistered(kFALSE),
  fCurrentEvent(0), fNEvents(0),
  fVertex(TVector3()), fVertexXYSigma(TVector2(0.42,0.36)), fTargetThickness(0.083),
  fBeamAngle(TVector2(-0.022,0.)), fBeamAngleABSigma(TVector2(0.035,0)), fIsRandomRP(kTRUE)
{
  fInputPath = TString::Format("%s/input/", gSystem->Getenv("VMCWORKDIR"));
  this -> RegisterReader();
}

STModelToLabFrameGenerator::STModelToLabFrameGenerator(TString filePath, TString fileName)
:FairGenerator("STModelToLabFrame",fileName),
  fReader(nullptr), fInputPath(filePath), fInputName(fileName),
  fB(-1.), 
  fBeamInfo(NULL), fHeavyIonsRegistered(kFALSE),
  fCurrentEvent(0), fNEvents(0),
  fVertex(TVector3()), fVertexXYSigma(TVector2(0.42,0.36)), fTargetThickness(0.043),
  fBeamAngle(TVector2(-0.022,0.)), fBeamAngleABSigma(TVector2(0.035,0)), fIsRandomRP(kTRUE)
{
  this -> RegisterReader();
}

STModelToLabFrameGenerator::~STModelToLabFrameGenerator()
{
}

void STModelToLabFrameGenerator::Print()
{
  auto info = TString::Format("Data info                 :\t%s\n"
                              "Beam energy per nucleon   :\t%.3g (GeV/c2)\n"
                              "Beam mass                 :\t%d\n"
                              "Beam charge               :\t%d\n"
                              "Target mass               :\t%d\n"
                              "Target thickness          :\t%.3g (cm)\n"
                              "Vertex mean XY            :\t%.3g\t%.3g (cm)\n"
                              "Vertex sigma XY           :\t%.3g\t%.3g (cm)\n"
                              "Vertex detector sigma XY  :\t%.3g\t%.3g (cm)\n"
                              "Beam angle mean AB        :\t%.3g\t%.3g (rad)\n"
                              "Beam angle sigma AB       :\t%.3g\t%.3g (rad)\n"
                              "Beam detector sigma AB    :\t%.3g\t%.3g (rad)\n",
                              (fReader)? fReader -> Print().Data() : "",
                              fBeamEnergyPerN,
                              fBeamMass,
                              fBeamCharge,
                              fTargetMass,
                              fTargetThickness,
                              fVertex.X(), fVertex.Y(),
                              fVertexXYSigma.X(), fVertexXYSigma.Y(),
                              fBeamDetectorVertexSigma.X(), fBeamDetectorVertexSigma.Y(),
                              fBeamAngle.X(), fBeamAngle.Y(),
                              fBeamAngleABSigma.X(), fBeamAngleABSigma.Y(),
                              fBeamDetectorABSigma.X(), fBeamDetectorABSigma.Y());
  TObjArray *tx = info.Tokenize("\n");
  for(int i = 0; i < tx->GetEntries(); ++i)
    LOG(INFO) << ((TObjString*) tx->At(i))->String() << FairLogger::endl;
}

void STModelToLabFrameGenerator::RegisterReader()
{
  //if(fInputName.BeginsWith("amd"))    { fReader = new STAMDReader(fInputPath + fInputName); } 
  //else if(fInputName.BeginsWith("urqmd"))  { fReader = new STUrQMDReader(fInputPath + fInputName); } 
  //else if(fInputName.BeginsWith("pBUU"))   { fReader = new STpBUUReader(fInputPath + fInputName); } 
  if(fInputName.BeginsWith("imqmd") || fInputName.BeginsWith("approx"))  { fReader = new STImQMDReader(fInputPath + fInputName); }  // approx data shares the same format as imqmd
  else if(fInputName.BeginsWith("pbuu")) { fReader = new STpBUUReader(fInputPath + fInputName); }
  else
    LOG(FATAL)<<"STModelToLabFrameGenerator cannot accept event files without specifying generator names."<<FairLogger::endl;
  LOG(INFO)<<"-I Opening file: "<<fInputName<<FairLogger::endl;

  fNEvents = fReader -> GetEntries();

  auto ioMan = FairRootManager::Instance();
}

Bool_t STModelToLabFrameGenerator::Init()
{
  TString treeName, partBranchName;
  if(!fReader)
    LOG(FATAL) << "Source reader is not being set up propertly!" << FairLogger::endl;

  auto EBeam = fBeamEnergyPerN*fBeamMass + fBeamMass*fNucleonMass;
  auto PBeam = sqrt(EBeam*EBeam - fBeamMass*fBeamMass*fNucleonMass*fNucleonMass);
  TLorentzVector vBeam(0, 0, PBeam, EBeam + fTargetMass*fNucleonMass);
  fBoostVector = vBeam.BoostVector();

  if(!fHeavyIonsRegistered) this -> RegisterHeavyIon();
  fReader -> SetEntry(fCurrentEvent);

  LOG(INFO) << "Generator configuration " << FairLogger::endl;
  this -> Print();
}


Bool_t STModelToLabFrameGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  std::vector<STTransportParticle> particleList;
  if(!fReader -> GetNext(particleList))
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

  // detected beam angle is different from the real beam angle
  Double_t detectedBeamAngleA = gRandom->Gaus(beamAngleA, fBeamDetectorABSigma.X());
  Double_t detectedBeamAngleB = gRandom->Gaus(beamAngleB, fBeamDetectorABSigma.Y());
  Double_t detectedVertexX = gRandom->Gaus(eventVertex.X(), fBeamDetectorVertexSigma.X());
  Double_t detectedVertexY = gRandom->Gaus(eventVertex.Y(), fBeamDetectorVertexSigma.Y());

  TRotation rotatedFrame;             // set rotation operator for ta.
  rotatedFrame.RotateY(beamAngleA);   // rotate by angle-A of beam with respect to Y axis
  TVector3 axisX(1,0,0);              // define local X axis
  axisX.Transform(rotatedFrame);      // X' axis in rotated frame.
  Double_t thetaInRotatedFrame = -1.*TMath::ACos(TMath::Sqrt( (1.+tanBeamA*tanBeamA)/(tanBeamA*tanBeamA+tanBeamB*tanBeamB+1) ));  // tb' (be careful about the sign!!)
  TRotation rotateInRotatedFrame;
  rotateInRotatedFrame.Rotate(thetaInRotatedFrame,axisX); // rotate by angle-B' of beam with respect to X' axis.


  auto nPart = particleList.size();
  auto event = (FairMCEventHeader*) primGen->GetEvent();
  if( event && !(event->IsSet()) ){
    event->SetEventID(fCurrentEvent + 1);
    event->MarkSet(kTRUE);
    event->SetVertex(TVector3(detectedVertexX, detectedVertexY, fVertex.Z()));
    event->SetRotX(detectedBeamAngleA);
    event->SetRotY(detectedBeamAngleB);
    event->SetRotZ(phiRP); 
    event->SetB(fB);
    event->SetNPrim(nPart);
    if(auto castedEvent = dynamic_cast<STFairMCEventHeader*>(event))
    {
      castedEvent -> SetBeamZ(fBeamCharge);
      castedEvent -> SetBeamA(fBeamMass);
      castedEvent -> SetEnergyPerNucleons(fBeamEnergyPerN);
    }
  }

  Int_t numDiscarded = 0;
  // set up lorentz vector to transform 
  Int_t tracksCounter = 0;

  // shuffle if maximum multiplicity is needed
  if(fMaxMult > 0) std::random_shuffle(particleList.begin(), particleList.end());
  std::vector<STTransportParticle> acceptedParticles;
  for(const auto& particle : particleList)
  {
    Int_t pdg = particle.pdg;
    auto particleData = Elements::PDGToParticleData(pdg);
    int charge = particleData -> Charge()/3;
    if(charge == 0 || (charge > fMaxZ && fMaxZ > 0))
      continue;

    if(fAllowedPDG.size() > 0)
      if(fAllowedPDG.find(pdg) == fAllowedPDG.end())
        continue;

    acceptedParticles.push_back(particle);
    if(acceptedParticles.size() >= fMaxMult && fMaxMult > 0) break;
  }

  for(const auto& particle : acceptedParticles)
  {
    auto particleData = Elements::PDGToParticleData(particle.pdg);
    auto mass = particleData -> Mass(); // It's already in GeV

    // transform the data to CM frame
    TVector3 p(particle.px, particle.py, particle.pz);
    p.SetPhi(p.Phi()+phiRP);  // random reaction plane orientation.

    TLorentzVector pCM(p.x(), p.y(), p.z(), sqrt(p.Mag2() + mass*mass));
    pCM.Boost(fBoostVector);
    p = pCM.Vect();

    p.RotateY(beamAngleA);    // rotate w.r.t Y axis
    p.Transform(rotateInRotatedFrame);

    primGen -> AddTrack(particle.pdg, p.X(), p.Y(), p.Z(), eventVertex.X(), eventVertex.Y(), eventVertex.Z());
  }
  // add dummy particles if there are no particles. 
  // it shoot in the wrong direction so it should not leave a signal in the detector
  // a work around for geant4 not accepting empty particle stack
  if(acceptedParticles.empty())
    primGen -> AddTrack(2212, 0, 0, 0, eventVertex.X(), eventVertex.Y(), eventVertex.Z());

  
  event -> SetNPrim((acceptedParticles.empty())? 1 : acceptedParticles.size());
  fCurrentEvent++;
  return kTRUE;
}

void STModelToLabFrameGenerator::RegisterHeavyIon(std::set<int> pdgList)
{
  std::vector<FairIon*> particleList;
  if(pdgList.empty())
    particleList = fReader -> GetParticleList();
  else
  {
    fAllowedPDG = pdgList;
    for(auto pdg : pdgList)
    {
      auto particleData = Elements::PDGToParticleData(pdg);
      auto z = int(particleData -> Charge()/3. + 0.5);
      auto a = int(particleData -> Mass()/Elements::kProtonMass + 0.5);
      if(a > 1) particleList.push_back(new FairIon(Form("%d",a)+Elements::symbol[z-1],z,a,z));
    }
  }

  for(auto ion : particleList)
    FairRunSim::Instance()->AddNewIon(ion);

  fHeavyIonsRegistered = true;
}


void STModelToLabFrameGenerator::SetMaxAllowedZ(int t_z)
{
  fMaxZ = t_z;
  LOG(INFO) << "Generator will discard all heavy ions with Z > " << fMaxZ << FairLogger::endl;
}

ClassImp(STModelToLabFrameGenerator);
