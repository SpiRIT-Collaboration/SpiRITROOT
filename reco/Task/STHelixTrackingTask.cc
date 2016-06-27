#include "STHelixTrackingTask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

ClassImp(STHelixTrackingTask)

STHelixTrackingTask::STHelixTrackingTask()
: STRecoTask("Helix Tracking Task", 1, false)
{
}

STHelixTrackingTask::STHelixTrackingTask(Bool_t persistence)
: STRecoTask("Helix Tracking Task", 1, persistence)
{
}

STHelixTrackingTask::~STHelixTrackingTask()
{
}

void STHelixTrackingTask::SetNumTracksLowLimit(Int_t limit) { fNumTracksLowLimit = limit; }

InitStatus STHelixTrackingTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fHitArray = (TClonesArray *) fRootManager -> GetObject("STHit");
  if (fHitArray == nullptr) {
    LOG(ERROR) << "Cannot find STHit array!" << FairLogger::endl;
    return kERROR;
  }
  
  fTrackArray = new TClonesArray("STHelixTrack", 100);
  fRootManager -> Register("STHelixTrack", "SpiRIT", fTrackArray, fIsPersistence);

  fHitClusterArray = new TClonesArray("STHitCluster", 100);
  fRootManager -> Register("STHitCluster", "SpiRIT", fHitClusterArray, fIsPersistence);

  fTrackFinder = new STHelixTrackFinder();

  return kSUCCESS;
}

void STHelixTrackingTask::Exec(Option_t *opt)
{
  fTrackArray -> Delete();
  fHitClusterArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  fTrackFinder -> BuildTracks(fHitArray, fTrackArray, fHitClusterArray);

  if (fTrackArray -> GetEntriesFast() < fNumTracksLowLimit) {
    fEventHeader -> SetIsBadEvent();
    LOG(INFO) << Space() << "Found less than " << fNumTracksLowLimit << " helix tracks. Bad event!" << FairLogger::endl;
    fTrackArray -> Delete();
    fHitClusterArray -> Delete();
    return;
  }

  LOG(INFO) << Space() << "STHelixTrack " << fTrackArray -> GetEntriesFast() << FairLogger::endl;
}
