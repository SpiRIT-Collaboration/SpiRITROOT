#include "STRecoTrack.hh"

ClassImp(STRecoTrack);

STRecoTrack::STRecoTrack()
{
  Clear();
}

void STRecoTrack::Clear(Option_t *option)
{
  STRecoTrackCand::Clear();

  fCharge = -2;
  fVertexID = -1;
  fPOCAVertex.SetXYZ(0,0,0);
  fPosKyotoL.SetXYZ(0,0,0);
  fPosKyotoR.SetXYZ(0,0,0);
  fPosKatana.SetXYZ(0,0,0);
  fClusterIDArray.clear();
}
