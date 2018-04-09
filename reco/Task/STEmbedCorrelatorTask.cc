#include "STEmbedCorrelatorTask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

#include "STEmbedTrack.hh"

#include <iostream>
using namespace std;

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
    return;

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

      for (auto iReco = 0; iReco < numReco; iReco++)
	{
	  auto recoTrack = (STRecoTrack *) fRecoTrackArray -> At(iReco);
	  if(recoTrack -> IsEmbed() == false)
	    continue;
	  if(recoTrack -> GetNumEmbedClusters() < 5)
	    continue;//less than 5 embed clusters are not significant tracks

	  TVector3 mom_reco = recoTrack -> GetMomentum();
	  mom_reco -= mom_mc;

	  if(iReco == 0)
	    {
	      min_mom = mom_reco.Mag();
	      mostprob_idx = iReco;
	    }
	  else if(mom_reco.Mag() < min_mom)
	    {
	      min_mom = mom_reco.Mag();
	      mostprob_idx = iReco;
	    }
	}


      //Always create embedTrack if there is MC track input
      //Failed correlation will give -999,-999,-999, momentum

      STEmbedTrack *embedTrack = new STEmbedTrack();
      embedTrack -> SetInitialTrack(MCTrack);	  

      if(mostprob_idx == -1)
	LOG(INFO) << Space() << "No track correlation found" << FairLogger::endl;
      else
	{
	  auto mostprobTrack = (STRecoTrack *) fRecoTrackArray -> At(mostprob_idx);
	  embedTrack -> SetFinalTrack(mostprobTrack);	 
	  num_corr++;
	}

      new ((*fEmbedTrackArray)[iMC]) STEmbedTrack(embedTrack);
    }

  LOG(INFO) << Space() << "STEmbedTrack "<< num_corr << FairLogger::endl;
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
