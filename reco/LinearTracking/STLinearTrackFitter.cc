/**
 * @author JungWoo Lee
 */

#include "STLinearTrackFitter.hh"
#include "STDebugLogger.hh"
#include <iostream>

using namespace std;

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
  TVector3 direction = fODRFitter -> GetDirection();
  if (direction.Z() < 0)
    direction = -direction;
  track -> SetDirection(direction);
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
  TVector3 direction = fODRFitter -> GetDirection();
  if (direction.Z() < 0)
    direction = -direction;
  track1 -> SetDirection(direction);
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

  fODRFitter -> SetMatrixA(track -> GetSumDistCXX(),
                           track -> GetSumDistCXY(),
                           track -> GetSumDistCZX(),
                           track -> GetSumDistCYY(),
                           track -> GetSumDistCYZ(),
                           track -> GetSumDistCZZ());

  fODRFitter -> SetWeightSum(track -> GetChargeSum());
  fODRFitter -> SetNumPoints(track -> GetNumHits() - track -> GetNumHitsRemoved());
}

void
STLinearTrackFitter::SetDataToODRFitter(STLinearTrack* track, STHit* hit)
{
  fODRFitter -> Reset();

  STLinearTrack trackTemp(track);
  trackTemp.AddHit(hit);

  fODRFitter -> SetCentroid(trackTemp.GetXCentroid(),
                            trackTemp.GetYCentroid(),
                            trackTemp.GetZCentroid());

  fODRFitter -> SetMatrixA(trackTemp.GetSumDistCXX(),
                           trackTemp.GetSumDistCXY(),
                           trackTemp.GetSumDistCZX(),
                           trackTemp.GetSumDistCYY(),
                           trackTemp.GetSumDistCYZ(),
                           trackTemp.GetSumDistCZZ());

  fODRFitter -> SetWeightSum(trackTemp.GetChargeSum());
  fODRFitter -> SetNumPoints(trackTemp.GetNumHits() - trackTemp.GetNumHitsRemoved());
}

void
STLinearTrackFitter::SetDataToODRFitter(STLinearTrack* track1, 
                                        STLinearTrack* track2)
{
  fODRFitter -> Reset();

  STLinearTrack trackTemp(track1);

  std::vector<STHit*> *hitBuffer = track2 -> GetHitPointerArray();
  Int_t numHits = hitBuffer -> size();
  for (Int_t iHit = 0; iHit < numHits; iHit++) {
    STHit* hit = hitBuffer -> at(iHit);
    trackTemp.AddHit(hit);
  }

  fODRFitter -> SetCentroid(trackTemp.GetXCentroid(),
                            trackTemp.GetYCentroid(),
                            trackTemp.GetZCentroid());

  fODRFitter -> SetMatrixA(trackTemp.GetSumDistCXX(),
                           trackTemp.GetSumDistCXY(),
                           trackTemp.GetSumDistCZX(),
                           trackTemp.GetSumDistCYY(),
                           trackTemp.GetSumDistCYZ(),
                           trackTemp.GetSumDistCZZ());

  fODRFitter -> SetWeightSum(trackTemp.GetChargeSum());
  fODRFitter -> SetNumPoints(trackTemp.GetNumHits() - trackTemp.GetNumHitsRemoved());
}

void 
STLinearTrackFitter::SortHits(STLinearTrack* track)
{
  std::vector<STHit*> *hitArray = track -> GetHitPointerArray();
  std::vector<Int_t> *idArray  = track -> GetHitIDArray();

  Int_t size = idArray -> size();
  idArray -> clear();

  STHitSortByTrackDirection sorting(track -> GetDirection(), track -> GetCentroid());
  std::sort(hitArray -> begin(), hitArray -> end(), sorting);

  for (Int_t i = 0; i < size; i++)
    idArray -> push_back(hitArray -> at(i) -> GetHitID());

  track -> SetVertex(0, GetClosestPointOnTrack(track, hitArray -> at(0)  ));
  track -> SetVertex(1, GetClosestPointOnTrack(track, hitArray -> back() ));

  track -> SetIsSorted(kTRUE);
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

Double_t
STLinearTrackFitter::PerpDistLine(STLinearTrack* track, TVector3 hitPos)
{
  TVector3 direction = track -> GetDirection();
  TVector3 centroid  = track -> GetCentroid();

  Double_t xC = ( direction.Z() * (hitPos.Y() - centroid.Y())
                - direction.Y() * (hitPos.Z() - centroid.Z()));
  Double_t yC = ( direction.X() * (hitPos.Z() - centroid.Z())
                - direction.Z() * (hitPos.X() - centroid.X()));
  Double_t zC = ( direction.Y() * (hitPos.X() - centroid.X())
                - direction.X() * (hitPos.Y() - centroid.Y()));

  return sqrt(xC*xC + yC*yC + zC*zC);
}

TVector3
STLinearTrackFitter::PerpPlane(STLinearTrack* track, STHit* hit) 
{
  return PerpPlane(track, hit -> GetPosition());
}

TVector3
STLinearTrackFitter::PerpPlane(STLinearTrack* track, TVector3 hitPos)
{
  TVector3 normal = track -> GetNormal();
  TVector3 centroid = track -> GetCentroid();

  Double_t dist = abs(normal * hitPos - normal * centroid) / sqrt(normal * normal);

  return dist * normal;
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

Double_t
STLinearTrackFitter::GetLengthOnTrack(STLinearTrack* track, STHit* hit)
{
  TVector3 direction = track -> GetDirection();
  TVector3 centroid = track -> GetCentroid();
  TVector3 v = centroid - hit -> GetPosition();

  return direction.Dot(v);
}
