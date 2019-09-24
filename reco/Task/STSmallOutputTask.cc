#include <map>
#include <algorithm>

#include "STSmallOutputTask.hh"
#include "STRecoTrack.hh"
#include "STVertex.hh"
#include "TError.h"

ClassImp(STSmallOutputTask);

STSmallOutputTask::STSmallOutputTask()
{
  LOG(DEBUG) << "Default Constructor of STSmallOutputTask" << FairLogger::endl;
  gInterpreter->GenerateDictionary("vector<TVector3>","TVector3.h");
}

STSmallOutputTask::~STSmallOutputTask()
{
  LOG(DEBUG) << "Destructor of STSmallOutputTask" << FairLogger::endl;
}

void STSmallOutputTask::SetOutputFile(const std::string& filename, const std::string& treename)
{
  fSmallOutput_ = std::unique_ptr<TFile>(new TFile(filename.c_str(), "RECREATE"));
}

InitStatus STSmallOutputTask::Init()
{
  if(STRecoTask::Init()==kERROR)
    return kERROR;

  LOG(DEBUG) << "Initilization of STSmallOutputTask" << FairLogger::endl;
  if(fSmallOutput_)
  {
    LOG(INFO) << "Setting up tree structure for the small output" << FairLogger::endl;
    fSTRecoTrack = static_cast<TClonesArray*>(fRootManager->GetObject("STRecoTrack"));
    fVATracks =    static_cast<TClonesArray*>(fRootManager->GetObject("VATracks"));
    fBDCVertex =   static_cast<TClonesArray*>(fRootManager->GetObject("BDCVertex"));
    fSTVertex =    static_cast<TClonesArray*>(fRootManager->GetObject("STVertex"));
    fSTEmbedTrack = static_cast<TClonesArray*>(fRootManager->GetObject("STEmbedTrack"));

    fSmallOutput_->cd();
    fSmallTree_ = new TTree("cbmsim", "", 99, fSmallOutput_.get());

    fSmallTree_->Branch("EvtData", &fData);
  }
  else LOG(INFO) << "No file is set for small output" << FairLogger::endl;
}

void STSmallOutputTask::Exec(Option_t* option)
{
  LOG(DEBUG) << "Exec of STSmallOutputTask" << FairLogger::endl;
  if(fSmallOutput_)
  {
    fData.Clear();
    if(fBDCVertex->GetEntries() > 0) fData.bdcVertex = static_cast<STVertex*>(fBDCVertex->At(0))->GetPos();
    if(fSTVertex->GetEntries() > 0) fData.tpcVertex = static_cast<STVertex*>(fSTVertex->At(0))->GetPos();

    // map that stores id of reco track
    // will be used to map VATracks back to STRecoTracks
    std::map<int, STRecoTrack*> RecoToVATracks;
    if(fVATracks)
    {
      fData.vaMultiplicity = fVATracks->GetEntries();
      for(int ii = 0; ii < fVATracks->GetEntries(); ++ii) 
      {
        auto bdc_track = static_cast<STRecoTrack*>(fVATracks->At(ii));
        RecoToVATracks[bdc_track->GetRecoID()] = bdc_track;
      }
    }

    if(fSTEmbedTrack)
    {
      for(int ii = 0; ii < fSTEmbedTrack->GetEntries(); ++ii)
      {
        auto embed_track = static_cast<STEmbedTrack*>(fSTEmbedTrack->At(ii));
        for(auto track : *embed_track->GetRecoTrackArray())
        {
          // given that RecoID and VA ID are aligned, the two IsEmbed array should always be identical
          auto embed_id = track->GetRecoID();
          fData.recoEmbedTag[embed_id] = true;
          fData.vaEmbedTag[embed_id] = true;
        }
      }
    }
 
    fData.multiplicity = fSTRecoTrack->GetEntries(); 
    for(int ii = 0; ii < fSTRecoTrack->GetEntries(); ++ii)
    {
      auto RecoTrack = static_cast<STRecoTrack*>(fSTRecoTrack->At(ii));
      fData.recoMomVec.push_back(RecoTrack->GetMomentumTargetPlane());
      fData.recodedx[ii] = RecoTrack->GetdEdxWithCut(0, 0.6);
      fData.recoPosPOCA[ii] = RecoTrack->GetPOCAVertex();
      fData.recoMom[ii] = RecoTrack->GetMomentumTargetPlane();
      fData.recoNRowClusters[ii] = RecoTrack->GetNumRowClusters();
      fData.recoNLayerClusters[ii] = RecoTrack->GetNumLayerClusters(); 
      fData.recoPosTargetPlane[ii] = RecoTrack->GetPosTargetPlane();
      fData.recoCharge[ii] = RecoTrack->GetCharge();
      fData.recodpoca[ii] = fData.recoPosPOCA[ii] - fData.tpcVertex;

      // construct BDC data with identical vector range
      int reco_id = RecoTrack->GetRecoID();
      auto it = RecoToVATracks.find(reco_id);
      // fill VA branches if data is found
      if(it != RecoToVATracks.end())
      {
        auto VATrack = it->second;
        fData.vadedx[ii] = VATrack->GetdEdxWithCut(0, 0.6);
        fData.vaPosPOCA[ii] = VATrack->GetPOCAVertex();
        fData.vaMom[ii] = VATrack->GetMomentumTargetPlane();
        fData.vaNRowClusters[ii] = VATrack->GetNumRowClusters();
        fData.vaNLayerClusters[ii] = VATrack->GetNumLayerClusters(); 
        fData.vaPosTargetPlane[ii] = VATrack->GetPosTargetPlane();
        fData.vaCharge[ii] = VATrack->GetCharge();
        fData.vadpoca[ii] = fData.vaPosPOCA[ii] - fData.tpcVertex;
      }
    }
    fSmallTree_->Fill();
    LOG(INFO) << Space() << "Saving event to file" << FairLogger::endl;
  }
}

void STSmallOutputTask::FinishTask()
{
  if(fSmallOutput_) 
  {
    LOG(INFO) << "Saving data to file " << fSmallOutput_->GetName() << FairLogger::endl;
    fSmallOutput_->cd();
    fSmallTree_->Write();
  }
}
