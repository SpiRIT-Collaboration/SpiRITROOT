#include "STTransformFrameTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

ClassImp(STTransformFrameTask);

STTransformFrameTask::STTransformFrameTask()
{ 
  fLogger = FairLogger::GetLogger(); 
  fCMVector = new TClonesArray("STVectorVec3");
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

  double EBeam = fEnergy*fMBeam + fMBeam*fNucleonMass;
  double PBeam = sqrt(EBeam*EBeam - fMBeam*fMBeam*fNucleonMass*fNucleonMass);
  TLorentzVector LV(0,0,-PBeam,EBeam);
  fVBeam = LV.BoostVector();
  fMass = (TClonesArray*) ioMan -> GetObject("Mass");
  fPDG = (TClonesArray*) ioMan -> GetObject("PDG");
  fData = (TClonesArray*) ioMan -> GetObject("STData");

  ioMan -> Register("CMVector", "ST", fCMVector, fIsPersistence);
  return kSUCCESS;
}

void STTransformFrameTask::SetBeamEnergyPerN(double energy) { fEnergy = energy; }
void STTransformFrameTask::SetBeamMass(int mass) { fMBeam = mass; }

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
  fCMVector -> Delete();
  auto data = (STData*) fData -> At(0);
  auto mass = (STVectorF*) fMass -> At(0);
  auto pdg = (STVectorI*) fPDG -> At(0);
  auto CMVec = new((*fCMVector)[0]) STVectorVec3();

  int npart = data -> multiplicity;
  for(int part = 0; part < npart; ++part)
  {
    auto& mom = data -> vaMom[part];
    if(mass -> fElements[part] > 0)
    {
      int ParticleZ = 1;
      if(pdg -> fElements[part] > 1000020000) ParticleZ = 2;
      TLorentzVector pCM(ParticleZ*mom.x(), ParticleZ*mom.y(), ParticleZ*mom.z(), sqrt(ParticleZ*ParticleZ*mom.Mag2() + fNucleonMass*fNucleonMass*(mass->fElements[part])*(mass->fElements[part])));
      pCM.Boost(fVBeam);
      CMVec -> fElements.emplace_back(pCM.Px(), pCM.Py(), pCM.Pz());
    }
    else CMVec->fElements.emplace_back(0,0,0);
  }
}

void STTransformFrameTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }

