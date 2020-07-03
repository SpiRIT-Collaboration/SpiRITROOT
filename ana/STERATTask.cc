#include "STERATTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TRandom.h"
#include "STVector.hh"
#include "TDatabasePDG.h"

#include <cmath>

ClassImp(STERATTask);

STERATTask::STERATTask()
{
  fLogger = FairLogger::GetLogger(); 
  fERAT = new TClonesArray("STVectorF");
  auto erat = new((*fERAT)[0]) STVectorF;
  erat -> fElements.push_back(0);
}

STERATTask::~STERATTask()
{}

InitStatus STERATTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  ioMan -> Register("ERAT", "ST", fERAT, fIsPersistence);

  return kSUCCESS;
}

void STERATTask::SetParContainers()
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

void STERATTask::Exec(Option_t *opt)
{
  double Et_expected = 0;
  double El_expected = 0;
  for(int i = 0; i < fSupportedPDG.size(); ++i)
    if(auto particle = TDatabasePDG::Instance() -> GetParticle(fSupportedPDG[i]))
    {
      double ParticleMass = particle -> Mass()*1000;
      int ntracks = static_cast<STVectorF*>(fProb -> At(i)) -> fElements.size();
      for(int itrack = 0; itrack < ntracks; ++itrack)
      {
        auto& P = static_cast<STVectorVec3*>(fCMVector -> At(i)) -> fElements[itrack];
        double prob = static_cast<STVectorF*>(fProb -> At(i)) -> fElements[itrack];
        double Ei = sqrt(ParticleMass*ParticleMass + P.Mag2());
        double pt = P.Perp();
        double pl = P.z();
        Et_expected += prob*pt*pt/(ParticleMass + Ei);
        El_expected += prob*pl*pl/(ParticleMass + Ei);
      }
    }
  static_cast<STVectorF*>(fERAT -> At(0)) -> fElements[0] = Et_expected / El_expected;
}
