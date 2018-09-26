#include "STRecoTrack.hh"

ClassImp(STRecoTrack);

STRecoTrack::STRecoTrack()
{
  Clear();
}

STRecoTrack::STRecoTrack(STRecoTrack *recoTrack)
{
  Clear();

  // STRecoTrackCand class members
  fIsEmbed = recoTrack -> IsEmbed();
  num_embed_clusters = recoTrack -> GetNumEmbedClusters();
  fPID = recoTrack -> GetPID();
  fPIDProbability = recoTrack -> GetPIDProbability();

  fMomentum = recoTrack -> GetMomentum();
  fMomentumTargetPlane = recoTrack -> GetMomentumTargetPlane();
  fPosTargetPlane = recoTrack -> GetPosTargetPlane();

  auto dedxPointArray = recoTrack -> GetdEdxPointArray();
  for (auto dedxPoint : *dedxPointArray)
    AdddEdxPoint(dedxPoint);

  fGenfitTrack = recoTrack -> GetGenfitTrack();
  fHelixTrack = recoTrack -> GetHelixTrack();

  // STRecoTrack class members
  fCharge = recoTrack -> GetCharge();
  fGFCharge = recoTrack -> GetGenfitCharge();
  fParentID = recoTrack -> GetParentID();
  fVertexID = recoTrack -> GetVertexID();
  fHelixID = recoTrack -> GetHelixID();
  SetPOCAVertex(recoTrack -> GetPOCAVertex());
  SetPosKyotoL(recoTrack -> GetPosKyotoL());
  SetPosKyotoR(recoTrack -> GetPosKyotoR());
  SetPosKatana(recoTrack -> GetPosKatana());
  SetPosNeuland(recoTrack -> GetPosNeuland());

  auto clusterIDArray = recoTrack -> GetClusterIDArray();
  for (auto cid : *clusterIDArray)
    AddClusterID(cid);

  fNumRowClusters = recoTrack -> GetNumRowClusters();
  fNumLayerClusters = recoTrack -> GetNumLayerClusters();
  fNumRowClusters90 = recoTrack -> GetNumRowClusters90();
  fNumLayerClusters90 = recoTrack -> GetNumLayerClusters90();

  fHelixChi2R = recoTrack -> GetHelixChi2R();
  fHelixChi2X = recoTrack -> GetHelixChi2X();
  fHelixChi2Y = recoTrack -> GetHelixChi2Y();
  fHelixChi2Z = recoTrack -> GetHelixChi2Z();

  fTrackLength = recoTrack -> GetTrackLength();
  fNDF = recoTrack -> GetNDF();
  fChi2 = recoTrack -> GetChi2();
  fChi2R = recoTrack -> GetChi2R();
  fChi2X = recoTrack -> GetChi2X();
  fChi2Y = recoTrack -> GetChi2Y();
  fChi2Z = recoTrack -> GetChi2Z();

  fRecoID = recoTrack -> GetRecoID();
}

void STRecoTrack::Clear(Option_t *option)
{
  STRecoTrackCand::Clear();

  fCharge = -2;
  fGFCharge = -2;
  fParentID = -1;
  fVertexID = -1;
  fHelixID = -1;
  fPOCAVertex.SetXYZ(0,0,0);
  fPosKyotoL.SetXYZ(0,0,0);
  fPosKyotoR.SetXYZ(0,0,0);
  fPosKatana.SetXYZ(0,0,0);
  fPosNeuland.SetXYZ(0,0,0);
  fClusterIDArray.clear();

  fNumRowClusters = 0;
  fNumLayerClusters = 0;
  fNumRowClusters90 = 0;
  fNumLayerClusters90 = 0;

  fHelixChi2R = 0;
  fHelixChi2X = 0;
  fHelixChi2Y = 0;
  fHelixChi2Z = 0;

  fTrackLength = 0;
  fNDF = 0;
  fChi2 = 0;
  fChi2R = 0;
  fChi2X = 0;
  fChi2Y = 0;
  fChi2Z = 0;

  fRecoID = -1;
}
