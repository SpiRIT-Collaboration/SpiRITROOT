#include <map>
#include <algorithm>

#include "STSmallOutputTask.hh"
#include "STRecoTrack.hh"
#include "STVertex.hh"
#include "STParticle.hh"
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

void STSmallOutputTask::SetOutputFile(const std::string& filename)
{fSmallOutput_ = std::unique_ptr<TFile>(new TFile(filename.c_str(), "RECREATE"));}

void STSmallOutputTask::SetRun(int runID){ fRunID = runID; }

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
    fSTParticle =  static_cast<TClonesArray*>(fRootManager->GetObject("STParticle"));
    fVAParticle =  static_cast<TClonesArray*>(fRootManager->GetObject("VAParticle"));
    // BeamInfo comes from GenfitVATask
    // It won;t be filled if VA Task is absent
    fBeamInfo = static_cast<STBeamInfo*>(fRootManager->GetObject("STBeamInfo"));

    fSmallOutput_->cd();
    fSmallTree_ = new TTree("cbmsim", "", 99, fSmallOutput_.get());

    fSmallTree_->Branch("runID", &fRunID);
    fSmallTree_->Branch("EvtData", &fData);
    fSmallTree_->Branch("eventID", &fEventID);
    fSmallTree_->Branch("eventType", &fEventType);
  }
  else LOG(INFO) << "No file is set for small output" << FairLogger::endl;

}

void STSmallOutputTask::Exec(Option_t* option)
{
  LOG(DEBUG) << "Exec of STSmallOutputTask" << FairLogger::endl;
  if(fSmallOutput_)
  {
    fData.ResetDefaultWithLength(fSTRecoTrack->GetEntries());
    fEventID = fEventHeader -> GetEventID();
    fEventType = fEventHeader -> GetStatus();
    if(fBeamInfo)
    {
      fData.aoq = fBeamInfo -> fBeamAoQ;
      fData.z = fBeamInfo -> fBeamZ;
      fData.a = fBeamInfo -> fRotationAngleA;
      fData.b = fBeamInfo -> fRotationAngleB;
      fData.proja = fBeamInfo -> fRotationAngleTargetPlaneA;
      fData.projb = fBeamInfo -> fRotationAngleTargetPlaneB;
      fData.projx = fBeamInfo -> fProjX;
      fData.projy = fBeamInfo -> fProjY;
      fData.beamEnergy = fBeamInfo -> fBeamEnergy;
      fData.beta = fBeamInfo -> fBeamVelocity;
    }

    if(fBDCVertex)
      if(fBDCVertex->GetEntries() > 0) 
        fData.bdcVertex = static_cast<STVertex*>(fBDCVertex->At(0))->GetPos();

    STVertex* stVertex = nullptr;
    if(fSTVertex)
      if(fSTVertex->GetEntries() > 0)
      {
        stVertex =  static_cast<STVertex*>(fSTVertex->At(0));
        fData.tpcVertex = stVertex->GetPos();
      }

    // map that stores id of reco track
    // will be used to map VATracks back to STRecoTracks
    std::map<int, STRecoTrack*> RecoToVATracks;
    std::map<int, STParticle*> RecoToVAParticle;
    if(fVATracks)
    {
      fData.vaMultiplicity = fVATracks->GetEntries();
      for(int ii = 0; ii < fVATracks->GetEntries(); ++ii) 
      {
        auto bdc_track = static_cast<STRecoTrack*>(fVATracks->At(ii));
        RecoToVATracks[bdc_track->GetRecoID()] = bdc_track;
      }
    }
    if(fVAParticle)
    {
      fData.vaMultiplicity = fVATracks->GetEntries();
      for(int ii = 0; ii < fVATracks->GetEntries(); ++ii) 
      {
        auto bdc_track = static_cast<STRecoTrack*>(fVATracks->At(ii));
        auto bdc_particle = static_cast<STParticle*>(fVAParticle->At(ii));
        RecoToVAParticle[bdc_track->GetRecoID()] = bdc_particle;
      }
    }
 
 
    fData.multiplicity = fSTRecoTrack->GetEntries(); 
    for(int ii = 0; ii < fSTRecoTrack->GetEntries(); ++ii)
    {
      auto RecoTrack = static_cast<STRecoTrack*>(fSTRecoTrack->At(ii));
      fData.recodedx[ii] = RecoTrack->GetdEdxWithCut(0, 0.6);
      fData.recoPosPOCA[ii] = RecoTrack->GetPOCAVertex();
      fData.recoMom[ii] = RecoTrack->GetMomentumTargetPlane();
      fData.recoNRowClusters[ii] = RecoTrack->GetNumRowClusters();
      fData.recoNLayerClusters[ii] = RecoTrack->GetNumLayerClusters(); 
      fData.recoPosTargetPlane[ii] = RecoTrack->GetPosTargetPlane();
      fData.recoCharge[ii] = RecoTrack->GetCharge();
      fData.recodpoca[ii] = fData.recoPosPOCA[ii] - fData.tpcVertex;

      if(fSTParticle)
      {
        auto particle = static_cast<STParticle*>(fSTParticle -> At(ii));
        fData.RecoPIDTight[ii] = particle -> GetPIDTight();
        fData.RecoPIDNormal[ii] = particle -> GetPIDNorm();
        fData.RecoPIDLoose[ii] = particle -> GetPIDLoose();
      }

      // construct BDC data with identical vector range
      int reco_id = RecoTrack->GetRecoID();
      auto it_track = RecoToVATracks.find(reco_id);
      auto it_particle = RecoToVAParticle.find(reco_id);
      // fill VA branches if data is found
      if(it_track != RecoToVATracks.end())
      {
        auto VATrack = it_track->second;
        fData.vadedx[ii] = VATrack->GetdEdxWithCut(0, 0.6);
        fData.vaPosPOCA[ii] = VATrack->GetPOCAVertex();
        fData.vaMom[ii] = VATrack->GetMomentumTargetPlane();
        fData.vaNRowClusters[ii] = VATrack->GetNumRowClusters();
        fData.vaNLayerClusters[ii] = VATrack->GetNumLayerClusters(); 
        fData.vaPosTargetPlane[ii] = VATrack->GetPosTargetPlane();
        fData.vaCharge[ii] = VATrack->GetCharge();
        fData.vadpoca[ii] = fData.vaPosPOCA[ii] - fData.tpcVertex;
      }

      if(it_particle != RecoToVAParticle.end())
      {
        auto particle = it_particle -> second;
        fData.VAPIDTight[ii] = particle -> GetPIDTight();
        fData.VAPIDNormal[ii] = particle -> GetPIDNorm();
        fData.VAPIDLoose[ii] = particle -> GetPIDLoose();
      }
    }

    // swith on embedding tag   if(fSTEmbedTrack)
    if(fSTEmbedTrack)
      for(int ii = 0; ii < fSTEmbedTrack->GetEntries(); ++ii)
      {
        auto embed_track = static_cast<STEmbedTrack*>(fSTEmbedTrack->At(ii));
        fData.embedMom = embed_track->GetInitialMom()*1000;
        for(auto track : *embed_track->GetRecoTrackArray())
        {
          // given that RecoID and VA ID are aligned, the two IsEmbed array should always be identical
          auto embed_id = track->GetRecoID();
          fData.recoEmbedTag[embed_id] = true;
          fData.vaEmbedTag[embed_id] = true;
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
