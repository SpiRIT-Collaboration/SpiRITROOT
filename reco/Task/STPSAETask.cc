#include "STPSAETask.hh"
#include "STPSAFastFit.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

ClassImp(STPSAETask)

STPSAETask::STPSAETask()
: STRecoTask("PSA Task", 1, false)
{
}

STPSAETask::STPSAETask(Bool_t persistence, Double_t threshold, Int_t layerLowCut, Int_t layerHighCut)
: STRecoTask("PSA Task", 1, persistence)
{
  fThreshold = threshold;
  fLayerLowCut = layerLowCut;
  fLayerHighCut = layerHighCut;
}

STPSAETask::~STPSAETask()
{
}

void STPSAETask::SetThreshold(Double_t threshold) { fThreshold = threshold; }

void STPSAETask::SetLayerCut(Int_t lowCut, Int_t highCut)
{
  fLayerLowCut = lowCut;
  fLayerHighCut = highCut;
}

InitStatus STPSAETask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fRawEventArray = (TClonesArray *) fRootManager -> GetObject("STRawEvent");
  if (fRawEventArray == nullptr) {
    LOG(ERROR) << "Cannot find STRawEvent array!" << FairLogger::endl;
    return kERROR;
  }

  fHitArray = new TClonesArray("STHit", 1000);
  fRootManager -> Register("STHit", "SpiRIT", fHitArray, fIsPersistence);

  fPSA = new STPSAFastFit();
  fPSA -> SetThreshold(fThreshold);
  fPSA -> SetLayerCut(fLayerLowCut, fLayerHighCut);

  return kSUCCESS;
}

void STPSAETask::Exec(Option_t *opt)
{
  fHitArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  STRawEvent *rawEvent = (STRawEvent *) fRawEventArray -> At(0);

  fPSA -> Analyze(rawEvent, fHitArray);

  if (fHitArray -> GetEntriesFast() < 1000) {
    fEventHeader -> SetIsBadEvent();
    LOG(INFO) << Space() << "Found less than 1000 hits. Bad event!" << FairLogger::endl;
    fHitArray -> Delete();
    return;
  }

  LOG(INFO) << Space() << "STHit " << fHitArray -> GetEntriesFast() << FairLogger::endl;
}
