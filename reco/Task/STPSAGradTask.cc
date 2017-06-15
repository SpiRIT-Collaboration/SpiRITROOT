#include "STPSAGradTask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

ClassImp(STPSAGradTask)

STPSAGradTask::STPSAGradTask()
: STRecoTask("PSA Task", 1, false)
{
}

STPSAGradTask::~STPSAGradTask()
{
}

void STPSAGradTask::SetThreshold(Double_t threshold) { fThreshold = threshold; }

void STPSAGradTask::SetLayerCut(Int_t lowCut, Int_t highCut)
{
  fLayerLowCut = lowCut;
  fLayerHighCut = highCut;
}

void STPSAGradTask::SetNumHitsLowLimit(Int_t limit) { fNumHitsLowLimit = limit; }

InitStatus STPSAGradTask::Init()
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

  fPSA = new STPSAGrad();
  fPSA -> SetThreshold(fThreshold);
  fPSA -> SetLayerCut(fLayerLowCut, fLayerHighCut);

  if (fRecoHeader != nullptr) {
    fRecoHeader -> SetPar("psa_threshold",       fThreshold);
    fRecoHeader -> SetPar("psa_layerLowCut",     fLayerLowCut);
    fRecoHeader -> SetPar("psa_layerHgihCut",    fLayerHighCut);
    fRecoHeader -> SetPar("psa_numHitsLowLimit", fNumHitsLowLimit);
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }

  return kSUCCESS;
}

void STPSAGradTask::Exec(Option_t *opt)
{
  fHitArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  STRawEvent *rawEvent = (STRawEvent *) fRawEventArray -> At(0);

  fPSA -> Analyze(rawEvent, fHitArray);

  if (fHitArray -> GetEntriesFast() < fNumHitsLowLimit) {
    fEventHeader -> SetIsBadEvent();
    LOG(INFO) << Space() << "Found less than " << fNumHitsLowLimit << " hits. Bad event!" << FairLogger::endl;
    fHitArray -> Delete();
    return;
  }

  LOG(INFO) << Space() << "STHit " << fHitArray -> GetEntriesFast() << FairLogger::endl;
}
