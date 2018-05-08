#include "STRecoTrack.hh"

ClassImp(STRecoTrack);

STRecoTrack::STRecoTrack()
{
  Clear();
}

// this class can be created with another object.
STRecoTrack::STRecoTrack(STRecoTrack* aTrack_tem)
{
//the below is for the STRecoTrackCand
  fPID = aTrack_tem->GetPID();
  fIsEmbed = aTrack_tem->IsEmbed();
  num_embed_clusters = aTrack_tem->GetNumEmbedClusters();
  fPIDProbability = aTrack_tem->GetPIDProbability();
  fMomentum= aTrack_tem->GetMomentum();
  fMomentumTargetPlane= aTrack_tem->GetMomentumTargetPlane();
  fPosTargetPlane = aTrack_tem->GetPosTargetPlane();
  fdEdxPointArray= *(aTrack_tem->GetdEdxPointArray());
  fGenfitTrack = aTrack_tem->GetGenfitTrack();
  fHelixTrack = aTrack_tem->GetHelixTrack();
  
//the below is for the STRecoTrackCand
  fCharge = aTrack_tem->GetCharge();
  fParentID = aTrack_tem->GetParentID(); 
  fVertexID = aTrack_tem->GetVertexID(); 
  fHelixID = aTrack_tem->GetHelixID();

  fPOCAVertex = aTrack_tem->GetPOCAVertex(); 
  fPosKyotoL = aTrack_tem->GetPosKyotoL(); 
  fPosKyotoR = aTrack_tem->GetPosKyotoR(); 
  fPosKatana = aTrack_tem->GetPosKatana(); 
  fPosNeuland = aTrack_tem->GetPosNeuland(); 

  fClusterIDArray = *(aTrack_tem->GetClusterIDArray()) ; 

  fEffCurvature1 = aTrack_tem->GetEffCurvature1();
  fEffCurvature2 = aTrack_tem->GetEffCurvature2();
  fEffCurvature3 = aTrack_tem->GetEffCurvature3();
}

void STRecoTrack::Clear(Option_t *option)
{
  STRecoTrackCand::Clear();

  fCharge = -2;
  fParentID = -1;
  fVertexID = -1;
  fHelixID = -1;
  fPOCAVertex.SetXYZ(0,0,0);
  fPosKyotoL.SetXYZ(0,0,0);
  fPosKyotoR.SetXYZ(0,0,0);
  fPosKatana.SetXYZ(0,0,0);
  fPosNeuland.SetXYZ(0,0,0);
  fClusterIDArray.clear();
}
