#include "STRiemannToHelixTask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

#include <iostream>
using namespace std;

ClassImp(STRiemannToHelixTask)

InitStatus STRiemannToHelixTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fFitter = new STHelixTrackFitter();

  fHitClusterArray = (TClonesArray *) fRootManager -> GetObject("STHitCluster");
  fRiemannArray = (TClonesArray *) fRootManager -> GetObject("STRiemannTrack");
  
  fHelixArray = new TClonesArray("STHelixTrack", 100);
  fRootManager -> Register("STHelixTrack", "SpiRIT", fHelixArray, fIsPersistence);

  return kSUCCESS;
}

void STRiemannToHelixTask::Exec(Option_t *opt)
{
  fHelixArray -> Delete();

  Int_t nRiemannTracks = fRiemannArray -> GetEntriesFast();
  for (auto iRiemann = 0; iRiemann < nRiemannTracks; ++iRiemann) {
    auto riemannTrack = (STRiemannTrack *) fRiemannArray -> At(iRiemann);
    auto riemannHits = riemannTrack -> GetHits();

    if (riemannHits->size() < 10)
      continue;

    Int_t idx = fHelixArray -> GetEntriesFast();
    auto helixTrack = new ((*fHelixArray)[idx]) STHelixTrack(idx);

    for (auto riemannHit : *riemannHits) {
      auto id = riemannHit -> GetHit() -> GetHitID();
      auto hitCluster = (STHitCluster *) fHitClusterArray -> At(id);

      helixTrack -> AddHit(hitCluster);
      helixTrack -> AddHitCluster(hitCluster);
    }
    fFitter -> FitCluster(helixTrack);
    helixTrack -> FinalizeClusters();
  }

  LOG(INFO) << Space() << "STHelixTrack " << fHelixArray -> GetEntriesFast() << FairLogger::endl;
}
