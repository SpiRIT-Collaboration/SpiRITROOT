#include "STTransformFrameTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// ROOT classes
#include "TDatabasePDG.h"

#include <iostream>

ClassImp(STTransformFrameTask);

STTransformFrameTask::STTransformFrameTask() : fTargetMass(0), fDoRotation(false)
{ 
  fLogger = FairLogger::GetLogger(); 
  fCMVector = new TClonesArray("STVectorVec3");
  fCMKE = new TClonesArray("STVectorVec3"); // the three compomenets in order are: transverse, long, total KE
  fFragRapidity = new TClonesArray("STVectorF");
  fBeamRapidity = new STVectorF();
}

STTransformFrameTask::~STTransformFrameTask()
{}

InitStatus STTransformFrameTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }
  fData = (TClonesArray*) ioMan -> GetObject("STData");

  ioMan -> Register("CMVector", "ST", fCMVector, fIsPersistence);
  ioMan -> Register("CMKE", "ST", fCMKE, fIsPersistence);
  ioMan -> Register("FragRapidity", "ST", fFragRapidity, fIsPersistence);
  ioMan -> Register("BeamRapidity", "ST", fBeamRapidity, fIsPersistence);

  fBeamRapidity -> fElements.push_back(0);
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    fCMVector -> ConstructedAt(i);
    fCMKE -> ConstructedAt(i);
    fFragRapidity -> ConstructedAt(i);
  }

  fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Target thickness is %f mm", fTargetThickness));
  return kSUCCESS;
}

void STTransformFrameTask::SetParContainers()
{
  FairRunAna *run = FairRunAna::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

void STTransformFrameTask::Exec(Option_t *opt)
{
  auto data = (STData*) fData -> At(0);
  int beamMass = (data -> aoq)*(data -> z) + 0.5;

  double energyLossInTarget = fEnergyLossInTarget.Eval(data -> beamEnergyTargetPlane)*fTargetThickness*1000/2.; // convert mm to microns
  double energyPerN = (data -> beamEnergyTargetPlane*beamMass - energyLossInTarget)/beamMass;
  data -> beamEnergy = energyPerN;
  double EBeam = energyPerN*beamMass + beamMass*fNucleonMass;
  double PBeam = sqrt(EBeam*EBeam - beamMass*beamMass*fNucleonMass*fNucleonMass);
  TLorentzVector LV(0,0,PBeam,EBeam);
  double beta = PBeam/(LV.Gamma()*beamMass*fNucleonMass + fTargetMass*fNucleonMass);
  auto vBeam = TVector3(0,0,-beta);
  fBeamRapidity -> fElements[0] = LV.Rapidity();

  // beam rotation
  TVector3 beamDirection(TMath::Tan(data -> proja/1000.), TMath::Tan(data ->projb/1000.),1.);
  beamDirection = beamDirection.Unit();
  auto rotationAxis = beamDirection.Cross(TVector3(0,0,1));
  auto rotationAngle = beamDirection.Angle(TVector3(0,0,1));

  int npart = data -> multiplicity;
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    int pdg = fSupportedPDG[i];
    int entry = fCMVector->GetEntriesFast();
    auto CMVector = static_cast<STVectorVec3*>(fCMVector -> At(i));
    auto CMKE = static_cast<STVectorVec3*>(fCMKE -> At(i));;
    auto FragRapidity = static_cast<STVectorF*>(fFragRapidity -> At(i));
    CMVector -> fElements.clear();
    CMKE -> fElements.clear();
    FragRapidity -> fElements.clear();

    for(int part = 0; part < npart; ++part)
      if(auto particle = TDatabasePDG::Instance()->GetParticle(pdg))
      {
        int ParticleZ = particle -> Charge()/3; // TParticlePDG define charge in units of |e|/3, probably to accomodate quarks
        double ParticleMass = particle -> Mass()*1000; // GeV to MeV

        auto mom = data -> vaMom[part]*ParticleZ;
        if(fDoRotation) mom.Rotate(rotationAngle, rotationAxis);

        TLorentzVector pCM(mom.x(), mom.y(), mom.z(), sqrt(mom.Mag2() + ParticleMass*ParticleMass));
        pCM.Boost(vBeam);
        CMVector -> fElements.emplace_back(pCM.Px(), pCM.Py(), pCM.Pz());

        double totKE = pCM.Energy() - ParticleMass;
        double transKE = pCM.Et() - ParticleMass;
        double longKE = totKE - transKE;
        CMKE -> fElements.emplace_back(transKE, longKE, totKE); 
        FragRapidity -> fElements.push_back(pCM.Rapidity()); 
      }
      else 
      {
        CMVector -> fElements.emplace_back(0,0,0);
        CMKE -> fElements.emplace_back(0,0,0);
        FragRapidity -> fElements.push_back(0);
      }
  }
}

void STTransformFrameTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STTransformFrameTask::SetTargetMass(int tgMass)                                                 { fTargetMass = tgMass; }
void STTransformFrameTask::SetDoRotation(bool doRotate)                                              { fDoRotation = true; }
void STTransformFrameTask::SetTargetThickness(double thickness)                                         { fTargetThickness = thickness; }

void STTransformFrameTask::SetEnergyLossFile(TString fileName, int model)
{
  TString loadFormat = "%lg ";
  for(int i = 1; i < model; ++i) loadFormat += "%*lg ";
  loadFormat += "%lg";
  fEnergyLossInTarget = TGraph(fileName, loadFormat);
}
