#include "STHelixTrackFitter.hh"

#include "STDebugLogger.hh"
#include <iostream>
using namespace std;

ClassImp(STHelixTrackFitter)

bool
STHelixTrackFitter::FitPlane(STHelixTrack *track)
{
  if (track -> GetNumHits() < 4)
    return false;

  fODRFitter -> Reset();

  fODRFitter -> SetCentroid(track -> GetXMean(),
                            track -> GetYMean(),
                            track -> GetZMean());

  fODRFitter -> SetMatrixA(track -> CovWXX(),
                           track -> CovWXY(),
                           track -> CovWZX(),
                           track -> CovWYY(),
                           track -> CovWYZ(),
                           track -> CovWZZ());

  fODRFitter -> SetWeightSum(track -> GetChargeSum());
  fODRFitter -> SetNumPoints(track -> GetNumHits());

  if (fODRFitter -> Solve() == false)
    return false;

  fODRFitter -> ChooseEigenValue(2);

  TVector3 normal = fODRFitter -> GetDirection();

  if (normal.Y() < 1.e-10) {
    track -> SetIsLine();
    fODRFitter -> ChooseEigenValue(0);
    track -> SetLineDirection(fODRFitter -> GetDirection());
    track -> SetRMSH(fODRFitter -> GetRMSLine());
    return true;
  }

  track -> SetIsPlane();
  track -> SetPlaneNormal(normal);
  track -> SetRMSH(fODRFitter -> GetRMSPlane());

  return true;
}

bool
STHelixTrackFitter::Fit(STHelixTrack *track)
{
  if (track -> GetNumHits() < 4)
    return false;

  Double_t scale = 1;
  Double_t trackLength = track -> TrackLength();
  Double_t meanCharge = track -> GetChargeSum()/track -> GetNumHits();

  if (trackLength < 500.) {
    Double_t scaleTrackLength = 0.5*(500. - trackLength)/500.;
    if (meanCharge < 500.)
      scaleTrackLength *= (1 - (250000. - meanCharge*meanCharge)/250000.);
    scale = 1 + scaleTrackLength;
  }

  fODRFitter -> Reset();

  Double_t xMean = track -> GetXMean();
  Double_t zMean = track -> GetZMean();
  Double_t xCov  = track -> GetXCov();
  Double_t zCov  = track -> GetZCov();
  Double_t RSR = 2 * sqrt(xCov + zCov);

  Double_t xMapMean = 0;
  Double_t yMapMean = 0;
  Double_t zMapMean = 0;

  auto hitArray = track -> GetHitArray();

  Double_t x = 0;
  Double_t z = 0;

  for (auto hit : *hitArray)
  {
    x = hit -> GetX() - xMean;
    z = hit -> GetZ() - zMean;
    Double_t w = hit -> GetCharge();
    w = TMath::Power(w, scale);

    Double_t rEff = sqrt(x*x + z*z) / (2*RSR);
    Double_t denominator = 1 + rEff*rEff;

    Double_t xMap = x / denominator;
    Double_t yMap = 2 * RSR * rEff * rEff / denominator;
    Double_t zMap = z / denominator;

    xMapMean += w * xMap;
    yMapMean += w * yMap;
    zMapMean += w * zMap;
  }

  Double_t weightSum = track -> GetChargeSum();
  xMapMean = xMapMean / weightSum;
  yMapMean = yMapMean / weightSum;
  zMapMean = zMapMean / weightSum;

  fODRFitter -> SetCentroid(xMapMean, yMapMean, zMapMean);
  TVector3 mapMean(xMapMean, yMapMean, zMapMean);

  for (auto hit : *hitArray)
  {
    x = hit -> GetX() - xMean;
    z = hit -> GetZ() - zMean;
    Double_t w = hit -> GetCharge();
    w = TMath::Power(w, scale);

    Double_t rEff = sqrt(x*x + z*z) / (2*RSR);
    Double_t denominator = 1 + rEff*rEff;

    Double_t xMap = x / denominator;
    Double_t yMap = 2 * RSR * rEff * rEff / denominator;
    Double_t zMap = z / denominator;

    fODRFitter -> AddPoint(xMap, yMap, zMap, w);
  }

  if (fODRFitter -> Solve() == false)
    return false;

  fODRFitter -> ChooseEigenValue(0); TVector3 uOnPlane = fODRFitter -> GetDirection();
  fODRFitter -> ChooseEigenValue(1); TVector3 vOnPlane = fODRFitter -> GetDirection();
  fODRFitter -> ChooseEigenValue(2); TVector3 nToPlane = fODRFitter -> GetDirection();

  if (std::abs(nToPlane.Y()) < 1.e-8) {
    track -> SetIsLine();
    return false;
  }

  TVector3 RSC = TVector3(0, RSR, 0);
  Double_t tCC = nToPlane.Dot(mapMean - RSC) / nToPlane.Mag2();
  TVector3 RCC = tCC * nToPlane + RSC;
  Double_t dCS = (RCC - RSC).Mag();
  Double_t RCR = sqrt(RSR*RSR - dCS*dCS);

  Double_t uConst = uOnPlane.Y() / sqrt(uOnPlane.Y()*uOnPlane.Y() + vOnPlane.Y()*vOnPlane.Y());
  Double_t vConst = sqrt(1-uConst*uConst);

  Double_t ref1 = uConst*uOnPlane.Y() + vConst*vOnPlane.Y();
  Double_t ref2 = uConst*uOnPlane.Y() - vConst*vOnPlane.Y();

  if (ref1 < 0) ref1 = -ref1;
  if (ref2 < 0) ref2 = -ref2;
  if (ref1 < ref2) vConst = -vConst;

  TVector3 toLouu = uConst * uOnPlane + vConst * vOnPlane;
  TVector3 louu = RCC + toLouu*RCR;
  TVector3 high = RCC - toLouu*RCR;

  TVector3 louuInvMap(louu.X()/(1-louu.Y()/(2*RSR)), 0, louu.Z()/(1-louu.Y()/(2*RSR)));
  TVector3 highInvMap(high.X()/(1-high.Y()/(2*RSR)), 0, high.Z()/(1-high.Y()/(2*RSR)));

  TVector3 FCC = 0.5 * (louuInvMap + highInvMap);

  Double_t xC = FCC.X() + xMean;
  Double_t zC = FCC.Z() + zMean;
  Double_t radius = 0.5 * (louuInvMap - highInvMap).Mag();

  if (radius > 1.e+8) {
    track -> SetIsLine();
    return false;
  }

  track -> SetHelixCenter(xC, zC);
  track -> SetHelixRadius(radius);

  track -> SetIsHelix();

  sort(hitArray -> begin(), hitArray -> end(), STHitSortY());

  TVector3 position0 = hitArray -> at(0) -> GetPosition();
  x = position0.X() - xC;
  z = position0.Z() - zC;
  Double_t y = 0;

  Double_t alphaInit = TMath::ATan2(z, x);
  TVector2 xAxis(x,z);
  TVector2 zAxis(-z,x);

  xAxis = xAxis.Unit();
  zAxis = zAxis.Unit();

  Double_t expA  = 0;
  Double_t expA2 = 0;
  Double_t expY  = 0;
  Double_t expAY = 0;

  Double_t alphaStack = 0;
  Double_t alphaLast = 0;

  Double_t alphaMin = alphaInit;
  Double_t alphaMax = alphaInit;

  for (auto hit : *hitArray)
  {
    x = hit -> GetX() - xC;
    y = hit -> GetY();
    z = hit -> GetZ() - zC;;

    TVector2 v(x,z);

    Double_t xRot = v*xAxis;
    Double_t zRot = v*zAxis;

    alphaLast = TMath::ATan2(zRot, xRot);
    if (alphaLast > .5*TMath::Pi() || alphaLast < -.5*TMath::Pi())
    {
      Double_t t0 = alphaLast;

      alphaLast += alphaStack;
      alphaStack += t0;

      xAxis = v;
      zAxis = TVector2(-v.Y(), v.X());
      xAxis = xAxis.Unit();
      zAxis = zAxis.Unit();
    }
    else
      alphaLast += alphaStack;

    alphaLast = alphaLast + alphaInit;

    Double_t w = hit -> GetCharge();

    expA  += w * alphaLast;
    expA2 += w * alphaLast * alphaLast;
    expY  += w * y;
    expAY += w * alphaLast * y;

    if (alphaLast < alphaMin)
      alphaMin = alphaLast;
    if (alphaLast > alphaMax)
      alphaMax = alphaLast;
  }

  track -> SetAlphaHead(alphaMin);
  track -> SetAlphaTail(alphaMax);

  expA  /= weightSum;
  expA2 /= weightSum;
  expY  /= weightSum;
  expAY /= weightSum;

  Double_t slope  = (expAY - expA*expY) / (expA2 - expA*expA);
  Double_t offset = (expA2*expY - expA*expAY) / (expA2 - expA*expA);

  if (std::isinf(slope)) {
    track -> SetIsLine();
    return false;
  }

  track -> SetAlphaSlope(slope);
  track -> SetYInitial(offset);

  Double_t Sx = 0;
  Double_t Sy = 0;
  for (auto hit : *hitArray)
  {
    TVector3 q = track -> Map(hit -> GetPosition());
    Sx += hit -> GetCharge() * q.X() * q.X();
    Sy += hit -> GetCharge() * q.Y() * q.Y();
  }

  Double_t rmsr = sqrt(Sx / (track -> GetChargeSum() * (1 - 3/hitArray -> size())));
  Double_t rmsy = sqrt(Sy / (track -> GetChargeSum() * (1 - 3/hitArray -> size())));

  track -> SetRMSW(rmsr);
  track -> SetRMSH(rmsy);

  return true;
}

bool
STHelixTrackFitter::FitCluster(STHelixTrack *track)
{
  if (track -> GetNumStableClusters() < 3)
    return false;

  fODRFitter -> Reset();

  auto clusterArray = track -> GetClusterArray();

  Double_t xMean = 0;
  Double_t zMean = 0;
  Double_t weightSum = 0;
  for (auto cluster : *clusterArray) 
  {
    if (cluster -> IsStable() == false)
      continue;

    Double_t w = cluster -> GetCharge();
    xMean += w * cluster -> GetX();
    zMean += w * cluster -> GetZ();
    weightSum += w;
  }
  xMean = xMean / weightSum;
  zMean = zMean / weightSum;

  Double_t xCov  = track -> GetXCov();
  Double_t zCov  = track -> GetZCov();
  Double_t RSR = 2 * sqrt(xCov + zCov);

  Double_t xMapMean = 0;
  Double_t yMapMean = 0;
  Double_t zMapMean = 0;

  Double_t x = 0;
  Double_t z = 0;

  for (auto cluster : *clusterArray)
  {
    if (cluster -> IsStable() == false)
      continue;

    x = cluster -> GetX() - xMean;
    z = cluster -> GetZ() - zMean;
    Double_t w = cluster -> GetCharge();

    Double_t rEff = sqrt(x*x + z*z) / (2*RSR);
    Double_t denominator = 1 + rEff*rEff;

    Double_t xMap = x / denominator;
    Double_t yMap = 2 * RSR * rEff * rEff / denominator;
    Double_t zMap = z / denominator;

    xMapMean += w * xMap;
    yMapMean += w * yMap;
    zMapMean += w * zMap;
  }

  xMapMean = xMapMean / weightSum;
  yMapMean = yMapMean / weightSum;
  zMapMean = zMapMean / weightSum;

  fODRFitter -> SetCentroid(xMapMean, yMapMean, zMapMean);
  TVector3 mapMean(xMapMean, yMapMean, zMapMean);

  for (auto cluster : *clusterArray)
  {
    if (cluster -> IsStable() == false)
      continue;

    x = cluster -> GetX() - xMean;
    z = cluster -> GetZ() - zMean;
    Double_t w = cluster -> GetCharge();

    Double_t rEff = sqrt(x*x + z*z) / (2*RSR);
    Double_t denominator = 1 + rEff*rEff;

    Double_t xMap = x / denominator;
    Double_t yMap = 2 * RSR * rEff * rEff / denominator;
    Double_t zMap = z / denominator;

    fODRFitter -> AddPoint(xMap, yMap, zMap, w);
  }

  if (fODRFitter -> Solve() == false)
    return false;

  fODRFitter -> ChooseEigenValue(0); TVector3 uOnPlane = fODRFitter -> GetDirection();
  fODRFitter -> ChooseEigenValue(1); TVector3 vOnPlane = fODRFitter -> GetDirection();
  fODRFitter -> ChooseEigenValue(2); TVector3 nToPlane = fODRFitter -> GetDirection();

  if (std::abs(nToPlane.Y()) < 1.e-10) {
    track -> SetIsLine();
    return false;
  }

  TVector3 RSC = TVector3(0, RSR, 0);
  Double_t tCC = nToPlane.Dot(mapMean - RSC) / nToPlane.Mag2();
  TVector3 RCC = tCC * nToPlane + RSC;
  Double_t dCS = (RCC - RSC).Mag();
  Double_t RCR = sqrt(RSR*RSR - dCS*dCS);

  Double_t uConst = uOnPlane.Y() / sqrt(uOnPlane.Y()*uOnPlane.Y() + vOnPlane.Y()*vOnPlane.Y());
  Double_t vConst = sqrt(1-uConst*uConst);

  Double_t ref1 = uConst*uOnPlane.Y() + vConst*vOnPlane.Y();
  Double_t ref2 = uConst*uOnPlane.Y() - vConst*vOnPlane.Y();

  if (ref1 < 0) ref1 = -ref1;
  if (ref2 < 0) ref2 = -ref2;
  if (ref1 < ref2) vConst = -vConst;

  TVector3 toLouu = uConst * uOnPlane + vConst * vOnPlane;
  TVector3 louu = RCC + toLouu*RCR;
  TVector3 high = RCC - toLouu*RCR;

  TVector3 louuInvMap(louu.X()/(1-louu.Y()/(2*RSR)), 0, louu.Z()/(1-louu.Y()/(2*RSR)));
  TVector3 highInvMap(high.X()/(1-high.Y()/(2*RSR)), 0, high.Z()/(1-high.Y()/(2*RSR)));

  TVector3 FCC = 0.5 * (louuInvMap + highInvMap);

  Double_t xC = FCC.X() + xMean;
  Double_t zC = FCC.Z() + zMean;
  Double_t radius = 0.5 * (louuInvMap - highInvMap).Mag();

  if (radius > 1.e+8 || std::isnan(radius)) {
    track -> SetIsLine();
    return false;
  }

  track -> SetHelixCenter(xC, zC);
  track -> SetHelixRadius(radius);

  track -> SetIsHelix();

  TVector3 position0;
  Int_t firstIndex = 0;
  for (auto cluster : *clusterArray) {
    if (cluster -> IsStable()) {
      position0 = cluster -> GetPosition();
      break;
    }
    firstIndex++;
  }

  x = position0.X() - xC;
  z = position0.Z() - zC;
  Double_t y = 0;

  Double_t alphaInit = TMath::ATan2(z, x);
  TVector2 xAxis(x,z);
  TVector2 zAxis(-z,x);

  xAxis = xAxis.Unit();
  zAxis = zAxis.Unit();

  Double_t expA  = 0;
  Double_t expA2 = 0;
  Double_t expY  = 0;
  Double_t expAY = 0;

  Double_t alphaStack = 0;
  Double_t alphaLast = 0;

  Double_t alphaMin = alphaInit;
  Double_t alphaMax = alphaInit;

  auto numClusters = clusterArray -> size();
  for (auto iCluster = firstIndex; iCluster < numClusters; iCluster++)
  {
    auto cluster = clusterArray -> at(iCluster);
    if (cluster -> IsStable() == false)
      continue;

    x = cluster -> GetX() - xC;
    y = cluster -> GetY();
    z = cluster -> GetZ() - zC;;

    TVector2 v(x,z);

    Double_t xRot = v*xAxis;
    Double_t zRot = v*zAxis;

    alphaLast = TMath::ATan2(zRot, xRot);
    if (alphaLast > .5*TMath::Pi() || alphaLast < -.5*TMath::Pi())
    {
      Double_t t0 = alphaLast;

      alphaLast += alphaStack;
      alphaStack += t0;

      xAxis = v;
      zAxis = TVector2(-v.Y(), v.X());
      xAxis = xAxis.Unit();
      zAxis = zAxis.Unit();
    }
    else
      alphaLast += alphaStack;

    alphaLast = alphaLast + alphaInit;

    Double_t w = cluster -> GetCharge();

    expA  += w * alphaLast;
    expA2 += w * alphaLast * alphaLast;
    expY  += w * y;
    expAY += w * alphaLast * y;

    if (alphaLast < alphaMin)
      alphaMin = alphaLast;
    if (alphaLast > alphaMax)
      alphaMax = alphaLast;
  }

  track -> SetAlphaHead(alphaMin);
  track -> SetAlphaTail(alphaMax);

  expA  /= weightSum;
  expA2 /= weightSum;
  expY  /= weightSum;
  expAY /= weightSum;

  Double_t slope  = (expAY - expA*expY) / (expA2 - expA*expA);
  Double_t offset = (expA2*expY - expA*expAY) / (expA2 - expA*expA);

  if (std::isinf(slope)) {
    track -> SetIsLine();
    return false;
  }

  track -> SetAlphaSlope(slope);
  track -> SetYInitial(offset);

  Double_t Sx = 0;
  Double_t Sy = 0;
  for (auto cluster : *clusterArray)
  {
    if (cluster -> IsStable() == false)
      continue;

    TVector3 q = track -> Map(cluster -> GetPosition());
    Sx += cluster -> GetCharge() * q.X() * q.X();
    Sy += cluster -> GetCharge() * q.Y() * q.Y();
  }

  Double_t rmsr = sqrt(Sx / (track -> GetChargeSum() * (1 - 3/clusterArray -> size())));
  Double_t rmsy = sqrt(Sy / (track -> GetChargeSum() * (1 - 3/clusterArray -> size())));

  track -> SetRMSW(rmsr);
  track -> SetRMSH(rmsy);

  return true;
}
