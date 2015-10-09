/**
 * @author JungWoo Lee
 */

#include "STLinearTrackFitter.hh"

STLinearTrackFitter::STLinearTrackFitter()
{
}

STLinearTrackFitter::~STLinearTrackFitter()
{
}

Double_t
STLinearTrackFitter::Fit(STLinearTrack* track)
{
  std::vector<STHit*> *hitPointerBuffer = track -> GetHitPointerArray();
  Int_t nHits = hitPointerBuffer -> size();

  for (Int_t iHit = 0; iHit < nHits; iHit++) {
    STHit* hit = hitPointerBuffer -> at(iHit);
  }

  TVirtualFitter::SetDefaultFitter("Minuit");
  fFitter = TVirtualFitter::Fitter(0, 6);
  fFitter -> SetFCN

}

Double_t
STLinearTrackFitter::Perp(STLinearTrack* track, STHit* hit) 
{
  TVector3 centroid = track -> GetCentroid();
  TVector3 direction = track -> GetNormal();
  TVector3 directionUnit = direciton.Unit();

  TVector3 hitPos = hit -> GetPosition();
  TVector3 hitPosMinusCentroid = hitPos - centroid;
  TVector3 hitPosMinusCentroidUnit = hitPosMinusCentroid.Unit();
  Double_t cosine = hitPosMinusCentroidUnit.Dot(directionUnit);

  return hitPosMinusCentroid.Mag()*sqrt(1 - cosine*cosine);
}

void 
STLinearTrackFitter::Merge(STLinearTrack* track, STHit* hit) 
{
}
