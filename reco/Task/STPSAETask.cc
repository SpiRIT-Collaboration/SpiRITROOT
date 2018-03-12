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
void STPSAETask::SetEmbedFile(TString filename){ fEmbedFile = filename; }
void STPSAETask::SetEmbedding(Bool_t value){ fIsEmbedding = value; }

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
  fRawEmbedEventArray = (TClonesArray *) fRootManager -> GetObject("STRawEmbedEvent");
  if (fRawEmbedEventArray == nullptr) {
    LOG(ERROR) << "Cannot find STRawEmbedEvent array!" << FairLogger::endl;
    return kERROR;
  }
  fRawDataEventArray = (TClonesArray *) fRootManager -> GetObject("STRawDataEvent");
  if (fRawDataEventArray == nullptr) {
    LOG(ERROR) << "Cannot find STRawDataEvent array!" << FairLogger::endl;
    return kERROR;
  }


  fHitArray = new TClonesArray("STHit", 1000);
  fRootManager -> Register("STHit", "SpiRIT", fHitArray, fIsPersistence);
  fEmbedHitArray = new TClonesArray("STHit", 1000);
  fRootManager -> Register("STEmbedHit", "SpiRIT", fEmbedHitArray, fIsPersistence);
  fDataHitArray = new TClonesArray("STHit", 1000);
  fRootManager -> Register("STDataHit", "SpiRIT", fDataHitArray, fIsPersistence);

  if (!fPulserDataName.IsNull())
    fPSA = new STPSAFastFit(fPulserDataName);
  else
    fPSA = new STPSAFastFit(fShapingTime);
  fPSA -> SetThreshold(fThreshold);
  fPSA -> SetLayerCut(fLayerLowCut, fLayerHighCut);
  fPSA -> SetGainMatchingScale(fGainMatchingScale);

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
  fEmbedHitArray -> Delete();
  fDataHitArray -> Delete();
  
  if (fEventHeader -> IsBadEvent())
    return;

  Double_t *tbOffsets = fEventHeader -> GetTbOffsets();
  fPSA -> SetTbOffsets(tbOffsets);

  STRawEvent *rawEvent = (STRawEvent *) fRawEventArray -> At(0);
  STRawEvent *rawDataEvent = (STRawEvent *) fRawDataEventArray -> At(0);
  STRawEvent *rawEmbedEvent = (STRawEvent *) fRawEmbedEventArray -> At(0);
  
  fPSA -> Analyze(rawEvent, fHitArray);
  fPSA -> Analyze(rawDataEvent, fDataHitArray);
  if(rawEmbedEvent != NULL)
    fPSA -> Analyze(rawEmbedEvent, fEmbedHitArray);
  
  //DO SOMETHING HERE??????????? with embed or real data events???
  if (fHitArray -> GetEntriesFast() < fNumHitsLowLimit) {
    fEventHeader -> SetIsBadEvent();
    LOG(INFO) << Space() << "Found less than " << fNumHitsLowLimit << " hits. Bad event!" << FairLogger::endl;
    fHitArray -> Delete();
    return;
  }

  auto CorrelateEmbedHits = [](TClonesArray *hitAry, TClonesArray *embedHitAry){
    Int_t num_embed = 0;
    for (int iHit = 0; iHit < hitAry->GetEntries(); iHit++)
      {
	STHit *hit = (STHit *)hitAry->At(iHit);
	for (int iEmb = 0; iEmb < embedHitAry->GetEntries(); iEmb++)
	  {
	    STHit *embedHit = (STHit *)embedHitAry->At(iEmb);    
	    bool tb_corr = false;
	    bool charge_corr = false;
	    
	    if(hit->GetLayer() == embedHit->GetLayer() && hit->GetRow() == embedHit->GetRow())
	      {
		double chg_f = (hit->GetCharge()-embedHit->GetCharge())/embedHit->GetCharge();//charge fraction
		double tb_f = abs(hit->GetTb()-embedHit->GetTb());//charge fraction
		if( chg_f < .05 && tb_f < 3)
		  {
		    hit->SetIsEmbed(true);
		    num_embed++;
		  }
	      }
	  }
      }

    return num_embed;
  };
    
  Int_t num_embed = 0;
  if(rawEmbedEvent != NULL)
    num_embed = CorrelateEmbedHits(fHitArray,fEmbedHitArray);

  LOG(INFO) << Space() << "Embeded Hits " << fEmbedHitArray->GetEntries() << FairLogger::endl;
  LOG(INFO) << Space() << "Correlated Hits " << num_embed << FairLogger::endl;
  LOG(INFO) << Space() << "STHit "<< fHitArray -> GetEntriesFast() << FairLogger::endl;
}

void STPSAETask::SetGainMatchingScale(Double_t val) { fGainMatchingScale = val; }
