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

void
STLinearTrackFitter::Fit(STLinearTrack* track, STHit* hit, 
                         Double_t &rmsL, Double_t &rmsP)
{
  SetDataToODRFitter(track, hit);
  fODRFitter -> Solve();

  fODRFitter -> ChooseEigenValue(0);
  rmsL = fODRFitter -> GetRMSLine();
  fODRFitter -> ChooseEigenValue(2);
  rmsP = fODRFitter -> GetRMSPlane();
}

void
STLinearTrackFitter::Fit(STLinearTrack* track1, STLinearTrack* track2,
                         Double_t &rmsL, Double_t &rmsP)
{
  SetDataToODRFitter(track1, track2);
  fODRFitter -> Solve();

  fODRFitter -> ChooseEigenValue(0);
  rmsL = fODRFitter -> GetRMSLine();
  fODRFitter -> ChooseEigenValue(2);
  rmsP = fODRFitter -> GetRMSPlane();
}

void 
STLinearTrackFitter::FitAndSetTrack(STLinearTrack* track)
{
  SetDataToODRFitter(track);
  fODRFitter -> Solve();

  fODRFitter -> ChooseEigenValue(0);
  track -> SetDirection(fODRFitter -> GetDirection());
  track -> SetRMSLine(fODRFitter -> GetRMSLine());

  fODRFitter -> ChooseEigenValue(2);
  track -> SetNormal(fODRFitter -> GetNormal());
  track -> SetRMSPlane(fODRFitter -> GetRMSPlane());

  track -> SetIsFitted(kTRUE);
}

void 
STLinearTrackFitter::MergeAndSetTrack(STLinearTrack* track1, STLinearTrack* track2)
{
  std::vector<STHit*> *hitPointerBuffer = track2 -> GetHitPointerArray();
  Int_t nHits = hitPointerBuffer -> size();
  for (Int_t iHit = 0; iHit < nHits; iHit++) {
    STHit* hit = hitPointerBuffer -> at(iHit);
    track1 -> AddHit(hit);
  }

  SetDataToODRFitter(track1);
  fODRFitter -> Solve();

  fODRFitter -> ChooseEigenValue(0);
  track1 -> SetDirection(fODRFitter -> GetDirection());
  track1 -> SetRMSLine(fODRFitter -> GetRMSLine());

  fODRFitter -> ChooseEigenValue(2);
  track1 -> SetNormal(fODRFitter -> GetNormal());
  track1 -> SetRMSPlane(fODRFitter -> GetRMSPlane());

  track1 -> SetIsFitted(kTRUE);
}

void
STLinearTrackFitter::SetDataToODRFitter(STLinearTrack* track)
{
  fODRFitter -> Reset();

  fODRFitter -> SetCentroid(track -> GetXCentroid(),
                            track -> GetYCentroid(),
                            track -> GetZCentroid());

  std::vector<STHit*> *hitPointerBuffer = track -> GetHitPointerArray();
  Int_t nHits = hitPointerBuffer -> size();

  for (Int_t iHit = 0; iHit < nHits; iHit++) {
    STHit* hitInTrack = hitPointerBuffer -> at(iHit);
    TVector3 position = hitInTrack -> GetPosition();
    fODRFitter -> AddPoint(position.X(), 
                           position.Y(), 
                           position.Z(), 
                           hitInTrack -> GetCharge());
  }
}

void
STLinearTrackFitter::SetDataToODRFitter(STLinearTrack* track, STHit* hit)
{
  fODRFitter -> Reset();

  TVector3 positionHit = hit -> GetPosition();
  Double_t chargeHit   = hit -> GetCharge();
  Double_t chargeSum   = track -> GetChargeSum();

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

  fODRFitter -> AddPoint(positionHit.X(), 
                         positionHit.Y(), 
                         positionHit.Z(), 
                         chargeHit);

  std::vector<STHit*> *hitPointerBuffer = track -> GetHitPointerArray();
  Int_t nHits = hitPointerBuffer -> size();

  for (Int_t iHit = 0; iHit < nHits; iHit++) {
    STHit* hitInTrack = hitPointerBuffer -> at(iHit);
    TVector3 position = hitInTrack -> GetPosition();
    fODRFitter -> AddPoint(position.X(), 
                           position.Y(), 
                           position.Z(), 
                           hitInTrack -> GetCharge());
  }
}

void
STLinearTrackFitter::SetDataToODRFitter(STLinearTrack* track1, 
                                        STLinearTrack* track2)
{
  fODRFitter -> Reset();

  STLinearTrack track;

  std::vector<STHit*> *hitPointerBuffer1 = track1 -> GetHitPointerArray();
  Int_t nHits1 = hitPointerBuffer1 -> size();
  for (Int_t iHit = 0; iHit < nHits1; iHit++) {
    STHit* hit = hitPointerBuffer1 -> at(iHit);
    track.AddHit(hit);
  }

  std::vector<STHit*> *hitPointerBuffer2 = track2 -> GetHitPointerArray();
  Int_t nHits2 = hitPointerBuffer2 -> size();
  for (Int_t iHit = 0; iHit < nHits2; iHit++) {
    STHit* hit = hitPointerBuffer2 -> at(iHit);
    track.AddHit(hit);
  }

  fODRFitter -> SetCentroid(track.GetXCentroid(),
                            track.GetYCentroid(),
                            track.GetZCentroid());

  for (Int_t iHit = 0; iHit < nHits1; iHit++) {
    STHit* hit = hitPointerBuffer1 -> at(iHit);
    TVector3 position = hit -> GetPosition();
    fODRFitter -> AddPoint(position.X(), 
                           position.Y(), 
                           position.Z(), 
                           hit -> GetCharge());
  }

  for (Int_t iHit = 0; iHit < nHits2; iHit++) {
    STHit* hit = hitPointerBuffer2 -> at(iHit);
    TVector3 position = hit -> GetPosition();
    fODRFitter -> AddPoint(position.X(), 
                           position.Y(), 
                           position.Z(), 
                           hit -> GetCharge());
  }
}

void 
STLinearTrackFitter::SortHits(STLinearTrack* track)
{
  std::vector<STHit*> *hitArray = track -> GetHitPointerArray();
  STHitSortByTrackDirection sorting(track -> GetDirection(), track -> GetCentroid());
  std::sort(hitArray -> begin(), hitArray -> end(), sorting);

  std::vector<Int_t> *idArray  = track -> GetHitIDArray();
  Int_t size = idArray -> size();
  idArray -> clear();
  for (Int_t i = 0; i < size; i++)
    idArray -> push_back(hitArray -> at(i) -> GetHitID());
}

TVector3
STLinearTrackFitter::GetClosestPointOnTrack(STLinearTrack* track, STHit* hit) 
{
  return GetClosestPointOnTrack(track, hit -> GetPosition());
}

TVector3
STLinearTrackFitter::GetClosestPointOnTrack(STLinearTrack* track, TVector3 hitPos)
{
  return (hitPos + PerpLine(track, hitPos));
}

TVector3
STLinearTrackFitter::PerpLine(STLinearTrack* track, STHit* hit) 
{
  return PerpLine(track, hit -> GetPosition());
}

TVector3
STLinearTrackFitter::PerpLine(STLinearTrack* track, TVector3 hitPos)
{
  TVector3 centroid = track -> GetCentroid();
  TVector3 directionUnit = track -> GetDirection();

  TVector3 hitPosMinusCentroid = hitPos - centroid;
  TVector3 hitPosMinusCentroidUnit = hitPosMinusCentroid.Unit();
  Double_t cosine = hitPosMinusCentroidUnit.Dot(directionUnit);

  directionUnit.SetMag(hitPosMinusCentroid.Mag()*cosine);

  return (directionUnit - hitPosMinusCentroid);
}

TVector3
STLinearTrackFitter::PerpPlane(STLinearTrack* track, STHit* hit) 
{
  return PerpPlane(track, hit -> GetPosition());
}

TVector3
STLinearTrackFitter::PerpPlane(STLinearTrack* track, TVector3 hitPos)
{
  TVector3 centroid = track -> GetCentroid();
  TVector3 normalUnit = track -> GetNormal();

  TVector3 hitPosMinusCentroid = hitPos - centroid;
  TVector3 hitPosMinusCentroidUnit = hitPosMinusCentroid.Unit();
  Double_t cosine = hitPosMinusCentroidUnit.Dot(normalUnit);

  normalUnit.SetMag(-hitPosMinusCentroid.Mag()*cosine);

  return normalUnit;
}

TVector3
STLinearTrackFitter::GetPointOnX(STLinearTrack* track, Double_t x)
{
  TVector3 direction = track -> GetDirection();
  TVector3 centroid = track -> GetCentroid();

  Double_t d = (x - centroid.X()) / direction.X();

  Double_t y = d * direction.Y() + centroid.Y();
  Double_t z = d * direction.Z() + centroid.Z();

  return TVector3(x,y,z);
}

TVector3
STLinearTrackFitter::GetPointOnY(STLinearTrack* track, Double_t y)
{
  TVector3 direction = track -> GetDirection();
  TVector3 centroid = track -> GetCentroid();

  Double_t d = (y - centroid.Y()) / direction.Y();

  Double_t x = d * direction.X() + centroid.X();
  Double_t z = d * direction.Z() + centroid.Z();

  return TVector3(x,y,z);
}

TVector3
STLinearTrackFitter::GetPointOnZ(STLinearTrack* track, Double_t z)
{
  TVector3 direction = track -> GetDirection();
  TVector3 centroid = track -> GetCentroid();

  Double_t d = (z - centroid.Z()) / direction.Z();

  Double_t x = d * direction.X() + centroid.X();
  Double_t y = d * direction.Y() + centroid.Y();

  return TVector3(x,y,z);
}
