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

void STCurveTrackingETask::SetNumTracksLowLimit(Int_t limit) { fNumTracksLowLimit = limit; }

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

  if (fTrackArray -> GetEntriesFast() < fNumTracksLowLimit) {
    fEventHeader -> SetIsBadEvent();
    LOG(INFO) << Space() << "Found less than " << fNumTracksLowLimit << " curve tracks. Bad event!" << FairLogger::endl;
    fTrackArray -> Delete();
    return;
  }

  LOG(INFO) << Space() << "STCurveTrack " << fTrackArray -> GetEntriesFast() << FairLogger::endl;
}
