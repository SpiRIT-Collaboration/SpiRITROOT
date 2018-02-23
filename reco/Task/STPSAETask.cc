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

STPSAETask::STPSAETask(Bool_t persistence, Int_t shapingTime, Double_t threshold, Int_t layerLowCut, Int_t layerHighCut)
: STRecoTask("PSA Task", 1, persistence)
{
  fThreshold = threshold;
  fShapingTime = shapingTime;
  fLayerLowCut = layerLowCut;
  fLayerHighCut = layerHighCut;
}


STPSAETask::STPSAETask(Bool_t persistence, TString pulserData, Double_t threshold, Int_t layerLowCut, Int_t layerHighCut)
: STRecoTask("PSA Task", 1, persistence)
{
  fThreshold = threshold;
  fPulserDataName = pulserData;
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

void STPSAETask::SetPulserData(TString pulserData) { fPulserDataName = pulserData; }
void STPSAETask::UseDefautPulserData(Int_t shapingTime) { fShapingTime = shapingTime; }

void STPSAETask::SetNumHitsLowLimit(Int_t limit) { fNumHitsLowLimit = limit; }

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

  if (!fPulserDataName.IsNull())
    fPSA = new STPSAFastFit(fPulserDataName);
  else
    fPSA = new STPSAFastFit(fShapingTime);
  fPSA -> SetThreshold(fThreshold);
  fPSA -> SetLayerCut(fLayerLowCut, fLayerHighCut);
  fPSA -> SetGainMatchingScale(fGainMatchingScale);
  fPSA -> SetGainMatchingData(fGainMatchingData);

  fShapingTime = fPSA -> GetShapingTime();

  if (fRecoHeader != nullptr) {
    fRecoHeader -> SetPar("psa_threshold",       fThreshold);
    fRecoHeader -> SetPar("psa_layerLowCut",     fLayerLowCut);
    fRecoHeader -> SetPar("psa_layerHgihCut",    fLayerHighCut);
    fRecoHeader -> SetPar("psa_pulserData",      fPulserDataName);
    fRecoHeader -> SetPar("psa_shapingTime",     fShapingTime);
    fRecoHeader -> SetPar("psa_numHitsLowLimit", fNumHitsLowLimit);
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }

  return kSUCCESS;
}

void STPSAETask::Exec(Option_t *opt)
{
  fHitArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  Double_t *tbOffsets = fEventHeader -> GetTbOffsets();
  fPSA -> SetTbOffsets(tbOffsets);

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

void STPSAETask::SetGainMatchingScale(Double_t val) { fGainMatchingScale = val; }
void STPSAETask::SetGainMatchingData(TString filename) { fGainMatchingData = filename; }
