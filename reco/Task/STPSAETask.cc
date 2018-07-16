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
void STPSAETask::SetPSAPeakFindingOption(Int_t opt) { fPSAPeakFindingOption = opt; }

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
    LOG(INFO) << "Cannot find STRawEmbedEvent array! Embedding disabled" << FairLogger::endl;
    fIsEmbedding = false;
  }
  fRawDataEventArray = (TClonesArray *) fRootManager -> GetObject("STRawDataEvent");
  if (fRawDataEventArray == nullptr) {
    LOG(INFO) << "Cannot find STRawDataEvent array! Embedding disabled" << FairLogger::endl;
    fIsEmbedding = false;
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
  fPSA -> SetGainMatchingData(fGainMatchingData);
  if (fYOffsetFileName)
    fPSA -> SetYOffsets(fYOffsetFileName);


  fShapingTime = fPSA -> GetShapingTime();
  fPSA -> SetPSAPeakFindingOption(fPSAPeakFindingOption);
  
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
  fPSA -> Analyze(rawEvent, fHitArray);

  if (fHitArray -> GetEntriesFast() < fNumHitsLowLimit) {
    fEventHeader -> SetIsBadEvent();
    LOG(INFO) << Space() << "Found less than " << fNumHitsLowLimit << " hits. Bad event!" << FairLogger::endl;
    fHitArray -> Delete();
    return;
  }

  
  if(fIsEmbedding == true)
    {
      STRawEvent *rawDataEvent = (STRawEvent *) fRawDataEventArray -> At(0);
      STRawEvent *rawEmbedEvent = (STRawEvent *) fRawEmbedEventArray -> At(0);
      fPSA -> Analyze(rawDataEvent, fDataHitArray);
      fPSA -> Analyze(rawEmbedEvent, fEmbedHitArray);
      
      //these vectors organize the hits by row and layer into this map structure
      //to avoid looping over the three arrays in a dumb way. Used in CorrelateEmbedHits
      //map index is row*112 + layer
      vector<vector<STHit *>> m_data (108*112,vector<STHit *>(0));   //just data hits
      vector<vector<STHit *>> m_embed(108*112,vector<STHit *>(0));   //just embeded hits
      vector<vector<STHit *>> m_hit  (108*112,vector<STHit *>(0));   //just data + embed hits
      
      for (int iHit = 0; iHit < fHitArray->GetEntries(); iHit++)
	{
	  STHit *hit = (STHit *)fHitArray->At(iHit);
	  m_hit.at(hit->GetRow()*112 + hit->GetLayer()).push_back(hit);
	}
      for (int iEmb = 0; iEmb < fEmbedHitArray->GetEntries(); iEmb++)
	{
	  STHit *embedHit = (STHit *)fEmbedHitArray->At(iEmb);
	  m_embed.at(embedHit->GetRow()*112 + embedHit->GetLayer()).push_back(embedHit);
	}
      for (int iData = 0; iData < fDataHitArray->GetEntries(); iData++)
	{
	  STHit *dataHit = (STHit *)fDataHitArray->At(iData);
	  m_data.at(dataHit->GetRow()*112 + dataHit->GetLayer()).push_back(dataHit);
	}
      int miss_hits = 0;
      auto CorrelateEmbedHits = [&miss_hits](vector<vector<STHit *>> map_hit, vector<vector<STHit *>> map_data, vector<vector<STHit *>> map_embed)
	{
	  
	  int num_embed = 0;
	  for (int iPad = 0; iPad < map_embed.size(); iPad++)
	    {
	      if(map_embed.at(iPad).size()!=0)
		{
		  miss_hits =  abs(map_hit.at(iPad).size() - map_embed.at(iPad).size() - map_data.at(iPad).size());
		  //	    bool any_missing_hits = (map_hit.at(i).size() + map_embed.at(i).size() != map_data.at(i).size());
		  
		  //First we find the hits that match just data without embeded hits
		  //Then we will correlate the remaining hits with the embeded hits
		  
		  //in the case there is more data than hits maybe the array of hits after errase becomes zero but the hit charge and hit _tb is not reset 
		  //consider loop over data ??
		  for(int iHit = 0; iHit < map_hit.at(iPad).size(); iHit++)
		    {
		      double hit_charge = map_hit.at(iPad).at(iHit)->GetCharge();
		      double hit_tb     = map_hit.at(iPad).at(iHit)->GetTb();
		      for(int iData = 0; iData < map_data.at(iPad).size(); iData++)
			{
			  double data_charge = map_data.at(iPad).at(iData)->GetCharge();
			  double data_tb     = map_data.at(iPad).at(iData)->GetTb();
			  
			  double chg_f = (hit_charge - data_charge)/data_charge;
			  double tb_diff = hit_tb - data_tb;
			  if( abs(chg_f) < .05 && abs(tb_diff) < 3) //condition for matching hit
			    {
			      map_hit.at(iPad).erase(map_hit.at(iPad).begin() + iHit);
			      //set to large values to not satisfy if condition above
			      //hit is correlated with data hit time to move to next iHit index
			      hit_charge = 999999;
			      hit_tb     = 999999;
			    }
			  
			}
		    }
		  miss_hits = (map_hit.at(iPad).size() - map_embed.at(iPad).size());
		  
		  //End of matching data hits
		  for(int iHit = 0; iHit < map_hit.at(iPad).size(); iHit++)
		    {
		      double hit_charge = map_hit.at(iPad).at(iHit)->GetCharge();
		      double hit_tb     = map_hit.at(iPad).at(iHit)->GetTb();
		      for(int iEmb = 0; iEmb< map_embed.at(iPad).size(); iEmb++)
			{
			  double embed_charge = map_embed.at(iPad).at(iEmb)->GetCharge();
			  double embed_tb     = map_embed.at(iPad).at(iEmb)->GetTb();
			  
			  double chg_f = (hit_charge - embed_charge)/embed_charge;
			  double tb_diff = hit_tb - embed_tb;
			  if( abs(tb_diff) < 3) //condition for matching hit
			    {
			      map_hit.at(iPad).at(iHit)->SetIsEmbed(true);
			      num_embed++;
			}
			}
		    }
		  //End of matching embeded hits
		}
	    }
	  //End of loop over pads in event
	  return num_embed;
	};
      
      Int_t num_embed = 0;
      num_embed = CorrelateEmbedHits(m_hit,m_data,m_embed);
      
      LOG(INFO) << Space() << "% Embed hits found " << (1.*num_embed)/(fEmbedHitArray->GetEntries()) << FairLogger::endl;
      //  LOG(INFO) << Space() << "Missing hits" << miss_hits << FairLogger::endl;
      //  LOG(INFO) << Space() << "Correlated Hits " << num_embed << FairLogger::endl;
    }
  LOG(INFO) << Space() << "STHit "<< fHitArray -> GetEntriesFast() << FairLogger::endl;
  
}

void STPSAETask::SetGainMatchingScale(Double_t val) { fGainMatchingScale = val; }
void STPSAETask::SetGainMatchingData(TString filename) { fGainMatchingData = filename; }
void STPSAETask::SetYOffsets(TString fileName) { fYOffsetFileName = fileName; }
