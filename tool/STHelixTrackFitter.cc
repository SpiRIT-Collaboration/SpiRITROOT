#include "STHelixTrackFitter.hh"

ClassImp(STHelixTrackFitter)

STHelixTrackFitter::STHelixTrackFitter()
{
  fODRFitter = new ODRFitter();
}

Bool_t 
STHelixTrackFitter::Fit(STHelixTrack *track)
{
  if (track -> GetNumHits() < 3)
    return kFALSE;

  FitCircle(track);
  FitLine(track);

  return kTRUE;
}

Bool_t
STHelixTrackFitter::FitCircle(STHelixTrack *track)
{
  fODRFitter -> Reset();

  Double_t xMean = track -> GetXMean();
  Double_t zMean = track -> GetZMean();
  Double_t xCov  = track -> GetXCov();
  Double_t zCov  = track -> GetZCov();
  Double_t weightSum = track -> GetChargeSum();

  Double_t RSR = 2 * sqrt(xCov + zCov);

  Double_t xMapMean = 0;
  Double_t yMapMean = 0;
  Double_t zMapMean = 0;

  std::vector<STHit *> *hitArray = track -> GetHitArray();

  for (auto *hit : *hitArray)
  {
    Double_t x = hit -> GetX() - xMean;
    Double_t z = hit -> GetZ() - zMean;
    Double_t w = hit -> GetCharge();

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

  for (auto *hit : *hitArray)
  {
    Double_t x = hit -> GetX() - xMean;
    Double_t z = hit -> GetZ() - zMean;
    Double_t w = hit -> GetCharge();

    Double_t rEff = sqrt(x*x + z*z) / (2*RSR);
    Double_t denominator = 1 + rEff*rEff;

    Double_t xMap = x / denominator;
    Double_t yMap = 2 * RSR * rEff * rEff / denominator;
    Double_t zMap = z / denominator;

    fODRFitter -> AddPoint(xMap, yMap, zMap, w);
  }

  fODRFitter -> Solve();
  fODRFitter -> ChooseEigenValue(0); TVector3 uOnPlane = fODRFitter -> GetDirection();
  fODRFitter -> ChooseEigenValue(1); TVector3 vOnPlane = fODRFitter -> GetDirection();
  fODRFitter -> ChooseEigenValue(2); TVector3 nToPlane = fODRFitter -> GetDirection();

  if (std::abs(nToPlane.Y()) < 1.e-10) {
    track -> SetFitStatus(STHelixTrack::kLine);
    //track -> SetLineDirection(nToPlane.Z(), 0, nToPlane.X());
    return kTRUE;
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

  Double_t xCenter = FCC.X() + xMean;
  Double_t zCenter = FCC.Z() + zMean;
  Double_t radius = 0.5 * (louuInvMap - highInvMap).Mag();

  track -> SetHelixCenter(xCenter, zCenter);
  track -> SetHelixRadius(radius);

  /*
  Double_t S = 0;
  for (auto *hit : *hitArray)
  {
    TVector3 position = hit -> GetPosition();
    Double_t d = radius - sqrt((position.X() - xCenter)*(position.X() - xCenter) + 
                               (position.Z() - zCenter)*(position.Z() - zCenter));
    S += hit -> GetCharge() * d * d;
  }
  rms = sqrt(S / (weightSum * (1 - 3/hitArray -> size())));
  */

  track -> SetFitStatus(STHelixTrack::kHelix);

  return kTRUE;
}

Bool_t
STHelixTrackFitter::FitLine(STHelixTrack *track)
{
  // TODO: Fit with wieight

  std::vector<STHit *> *hitArray = track -> GetHitArray();

  Double_t xC = track -> GetHelixCenterX();
  Double_t zC = track -> GetHelixCenterZ();
  Double_t r = track -> GetHelixRadius();

  STHitSortY sorting;
  sort(hitArray -> begin(), hitArray -> end(), sorting);

  TVector3 position0 = hitArray -> at(0) -> GetPosition();
  Double_t x = position0.X();
  Double_t y = position0.Y();
  Double_t z = position0.Z();

  Double_t alphaInit = TMath::ATan2(z, x);
  TVector2 xAxis(x,z);
  TVector2 zAxis(-z,x);

  xAxis = xAxis.Unit();
  zAxis = zAxis.Unit();

  Double_t alphaStack = 0;

  Double_t expA  = 0;
  Double_t expA2 = 0;
  Double_t expY  = 0;
  Double_t expAY = 0;

  for (auto *hit : *hitArray)
  {
    x = hit -> GetX();
    y = hit -> GetY();
    z = hit -> GetZ();

    TVector2 v(x,z);

    Double_t xRot = v*xAxis;
    Double_t zRot = v*zAxis;
    Double_t alpha = TMath::ATan2(zRot, xRot);

    if (alpha > TMath::Pi()/2. || alpha < -TMath::Pi()/2.)
    {
      Double_t t0 = alpha;

      alpha += alphaStack;
      alphaStack += t0;

      xAxis = v;
      zAxis = TVector2(-v.Y(), v.X());
      xAxis = xAxis.Unit();
      zAxis = zAxis.Unit();
    }
    else
      alpha += alphaStack;

    alpha = alpha + alphaInit;

    //TODO : weight

    expA  += alpha;
    expA2 += alpha * alpha;
    expY  += y;
    expAY += alpha*y;
  }

  Double_t numHits = hitArray -> size();

  expA  *= expA/numHits;
  expA2 *= expA2/numHits;
  expY  *= expA/numHits;
  expAY *= expAY/numHits;

  Double_t slope  = (expAY - expA*expY) / (expA2 - expA*expA);
  Double_t offset = (expA2*expY - expA*expAY) / (expA2 - expA*expA);
  Double_t dip = TMath::ATan(-slope/track -> GetHelixRadius()) + TMath::Pi()/2;

  //track -> SetAlphaSlope(slope);
  //track -> SetAlphaOffset(offset);
  track -> SetDipAngle(dip);

  return kTRUE;
}
