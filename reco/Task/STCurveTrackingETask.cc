#include "STCurveTrackingETask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

ClassImp(STCurveTrackingETask)

STCurveTrackingETask::STCurveTrackingETask()
: STRecoTask("Curve Tracking Task", 1, false)
{
}

STCurveTrackingETask::STCurveTrackingETask(Bool_t persistence)
: STRecoTask("Curve Tracking Task", 1, persistence)
{
}

STCurveTrackingETask::~STCurveTrackingETask()
{
}

InitStatus STCurveTrackingETask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fHitArray = (TClonesArray *) fRootManager -> GetObject("STHit");
  if (fHitArray == nullptr) {
    LOG(ERROR) << "Cannot find STHit array!" << FairLogger::endl;
    return kERROR;
  }
  
  fTrackArray = new TClonesArray("STCurveTrack", 100);
  fRootManager -> Register("STCurveTrack", "SpiRIT", fTrackArray, fIsPersistence);

  fTrackFinder = new STCurveTrackFinder();

  return kSUCCESS;
}

void STCurveTrackingETask::Exec(Option_t *opt)
{
  fTrackArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  fTrackFinder -> BuildTracks(fHitArray, fTrackArray);

  if (fTrackArray -> GetEntriesFast() < 5) {
    fEventHeader -> SetIsBadEvent();
    LOG(INFO) << Space() << "Found less than 5 curve tracks. Bad event!" << FairLogger::endl;
    fTrackArray -> Delete();
    return;
  }

  LOG(INFO) << Space() << "STCurveTrack " << fTrackArray -> GetEntriesFast() << FairLogger::endl;
}
