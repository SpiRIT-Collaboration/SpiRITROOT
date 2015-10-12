/**
 * @author JungWoo Lee
 */

#include "STLinearTrackFitter.hh"
#include "STDebugLogger.hh"

STLinearTrackFitter::STLinearTrackFitter()
{
  fODRFitter = new ODRFitter(); 
}

STLinearTrackFitter::~STLinearTrackFitter()
{
}

Double_t
STLinearTrackFitter::Fit(STLinearTrack* track)
{
  fODRFitter -> Reset();

  track -> GetXCentroid();


  fODRFitter -> SetCentroid(track -> GetXCentroid(),
                         track -> GetYCentroid(),
                         track -> GetZCentroid());


  std::vector<STHit*> *hitPointerBuffer = track -> GetHitPointerArray();
  Int_t nHits = hitPointerBuffer -> size();

  for (Int_t iHit = 0; iHit < nHits; iHit++) 
  {
    STHit* hit = hitPointerBuffer -> at(iHit);
    TVector3 position = hit -> GetPosition();
    fODRFitter -> AddPoint(position.X(), 
                        position.Y(), 
                        position.Z(), 
                        hit -> GetCharge());
  }

  fODRFitter -> FitLine();

  return fODRFitter -> GetRMS();
}

Double_t
STLinearTrackFitter::Fit(STLinearTrack* track, STHit* hit)
{
  fODRFitter -> Reset();

  TVector3 positionHit = hit -> GetPosition();
  Double_t chargeHit   = hit -> GetCharge();
  Double_t chargeSum  = track -> GetChargeSum();

  Double_t xC = chargeSum * track -> GetXCentroid()
              + chargeHit * positionHit.X();
  Double_t yC = chargeSum * track -> GetYCentroid()
              + chargeHit * positionHit.Y();
  Double_t zC = chargeSum * track -> GetZCentroid()
              + chargeHit * positionHit.Z();

  chargeSum += chargeHit;

  xC *= 1./chargeSum;
  yC *= 1./chargeSum;
  zC *= 1./chargeSum;

  fODRFitter -> SetCentroid(xC, yC, zC);

  std::vector<STHit*> *hitPointerBuffer = track -> GetHitPointerArray();
  Int_t nHits = hitPointerBuffer -> size();

  for (Int_t iHit = 0; iHit < nHits; iHit++) 
  {
    STHit* hitInTrack = hitPointerBuffer -> at(iHit);
    TVector3 position = hitInTrack -> GetPosition();
    fODRFitter -> AddPoint(position.X(), 
                        position.Y(), 
                        position.Z(), 
                        hitInTrack -> GetCharge());
  }

  fODRFitter -> AddPoint(positionHit.X(), 
                      positionHit.Y(), 
                      positionHit.Z(), 
                      hit -> GetCharge());

  fODRFitter -> FitLine();

  return fODRFitter -> GetRMS();
}

TVector3
STLinearTrackFitter::Perp(STLinearTrack* track, STHit* hit) 
{
  TVector3 hitPos = hit -> GetPosition();
  return Perp(track, hitPos);
}

TVector3
STLinearTrackFitter::Perp(STLinearTrack* track, TVector3 hitPos)
{
  TVector3 centroid = track -> GetCentroid();
  TVector3 directionUnit = track -> GetNormal();

  TVector3 hitPosMinusCentroid = hitPos - centroid;
  TVector3 hitPosMinusCentroidUnit = hitPosMinusCentroid.Unit();
  Double_t cosine = hitPosMinusCentroidUnit.Dot(directionUnit);

  TVector3 vectorAlongDirection = directionUnit;
  vectorAlongDirection.SetMag(hitPosMinusCentroid.Mag()*cosine);

  return (vectorAlongDirection - hitPosMinusCentroid);
}

TVector3
STLinearTrackFitter::GetClosestPointOnTrack(STLinearTrack* track, STHit* hit) 
{
  TVector3 hitPos = hit -> GetPosition();
  return GetClosestPointOnTrack(track, hitPos);
}

TVector3
STLinearTrackFitter::GetClosestPointOnTrack(STLinearTrack* track, TVector3 hitPos)
{
  TVector3 displacement = Perp(track, hitPos);
  return (hitPos + displacement);
}

void 
STLinearTrackFitter::FitAndSetTrack(STLinearTrack* track)
{
  Fit(track);
  track -> SetNormal(fODRFitter -> GetNormal());
  track -> SetIsFitted(kTRUE);
}
