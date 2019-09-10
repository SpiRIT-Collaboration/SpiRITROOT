#include "STEmbedCorrelatorTask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

#include "STEmbedTrack.hh"

#include <iostream>
using namespace std;

struct EmbedTrack {
  STRecoTrack *reco;
  double fract;
  int numEmbed;
};

ClassImp(STEmbedCorrelatorTask)

STEmbedCorrelatorTask::STEmbedCorrelatorTask()
{
  fIsPersistence = false;
}

STEmbedCorrelatorTask::~STEmbedCorrelatorTask()
{
}

InitStatus STEmbedCorrelatorTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fRecoTrackArray = (TClonesArray*) fRootManager -> GetObject("STRecoTrack");
  if (fRecoTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find RecoTrack array!" << FairLogger::endl;
    return kERROR;
  }
  fMCTrackArray = (TClonesArray*) fRootManager -> GetObject("STMCTrack");
  if (fMCTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find MCTrack array!" << FairLogger::endl;
    return kERROR;
  }
  
  fEmbedTrackArray = new TClonesArray("STEmbedTrack");
  fRootManager -> Register("STEmbedTrack", "SpiRIT", fEmbedTrackArray, fIsPersistence);

  return kSUCCESS;
}

void STEmbedCorrelatorTask::Exec(Option_t *opt)
{
  fEmbedTrackArray -> Delete();
  
  if (fEventHeader -> IsBadEvent())
    {
      STEmbedTrack *embedTrack = (STEmbedTrack *) fEmbedTrackArray -> ConstructedAt(0);
      return;
    }

  if(fMCTrackArray == NULL)
    return;
  
  auto numReco = fRecoTrackArray -> GetEntries();
  auto numMC   = fMCTrackArray -> GetEntries();

  
  int num_corr = 0;// number of tracks correlated
  
  for (auto iMC = 0; iMC < numMC; iMC++)
    {
      auto MCTrack = (STMCTrack *) fMCTrackArray -> At(iMC);
      TVector3 mom_mc(-999,-999,-999);
      MCTrack -> GetMomentum(mom_mc);
      mom_mc *= 1000;
      int mostprob_idx = -1;                //most probable index of reco track array
      double min_mom = 999999;              //minimum momentum magnitude difference

      std::vector<STRecoTrack *> *recotrack_ary = new std::vector<STRecoTrack *>(0);
      //      std::vector< std::pair<STRecoTrack *, Double_t>> pair_vec;
      std::vector<EmbedTrack> e_track_vec;
      
      for (auto iReco = 0; iReco < numReco; iReco++)
	{
	  auto recoTrack = (STRecoTrack *) fRecoTrackArray -> At(iReco);
	  //BDC reconstructed tracks(vaTracs) have the recoID set and we match to iReco
	  //position in array. Since we only copy embed tracks in this class we need to save recoID pos
	  recoTrack -> SetRecoID(iReco);
	

	  double fract_embed_clusters = static_cast<double>(recoTrack -> GetNumEmbedClusters())/static_cast<double>(recoTrack -> GetNumRowClusters() + recoTrack -> GetNumLayerClusters());

	  if(recoTrack -> IsEmbed() == false)
	    continue;
	  //less than 5 embed clusters are not significant tracks
	  if(recoTrack -> GetNumEmbedClusters() < 5)
	    continue;
	  //<50% embed clusters not a significant embeded track. Mixed with too many other clusters from real tracks.
	  if(fract_embed_clusters < .5 )
	    continue;

	  EmbedTrack e_track;
	  e_track.reco     = recoTrack;
	  e_track.fract    = fract_embed_clusters;
	  e_track.numEmbed = recoTrack->GetNumEmbedClusters();

	  e_track_vec.push_back(e_track);
	}

 
      auto comp_fcn = [] (EmbedTrack &a, EmbedTrack &b)
	{ return ( (a.fract > b.fract) && (a.numEmbed > b.numEmbed) ); };

      for(auto el : e_track_vec)
	{
	  recotrack_ary -> push_back(el.reco);
	  num_corr++;
	}

      //Always create embedTrack if there is MC track input
      //Failed correlation will give -999,-999,-999, momentum

      STEmbedTrack *embedTrack = (STEmbedTrack *) fEmbedTrackArray -> ConstructedAt(iMC);
      embedTrack -> SetInitialTrack(MCTrack);	  
      embedTrack -> SetRecoTrackArray(recotrack_ary);
      LOG(INFO) << Space() << "STEmbedTrack "<< recotrack_ary -> size()  << FairLogger::endl;
    }
}

Bool_t STEmbedCorrelatorTask::CheckMomCorr(STMCTrack *mctrack, STRecoTrack *recotrack)
{
  Bool_t IsCorr = false;
  TVector3 mc_mom(-999,-999,-999);
  mctrack->GetMomentum(mc_mom);
  mc_mom *= 1000; //MCTrack mom is in GeV/c and reco is MeV/c
  LOG(INFO) << Space() << "MC mom" << mc_mom.Px()<<" "<<mc_mom.Py()<<" "<<mc_mom.Pz()<< FairLogger::endl;
  TVector3 reco_mom = recotrack->GetMomentum();
  LOG(INFO) << Space() << "reco  mom" << reco_mom.Px()<<" "<<reco_mom.Py()<<" "<<reco_mom.Pz()<< FairLogger::endl;
  double x_diff = (mc_mom.Px()  - reco_mom.Px())/mc_mom.Px(); //Percent difference from expected
  double y_diff = (mc_mom.Py()  - reco_mom.Py())/mc_mom.Py();
  double z_diff = (mc_mom.Pz()  - reco_mom.Pz())/mc_mom.Pz();

  IsCorr = (abs(x_diff) < .02 && abs(y_diff) < .02 && abs(z_diff) < .02);
  
  LOG(INFO) << Space() << "Debug" << abs(x_diff)<<" "<<abs(y_diff)<<" "<<abs(z_diff)<< FairLogger::endl;
	      
  return IsCorr;
}
