#include "STRiemannFitter.hh"

ClassImp(STRiemannFitter)

STRiemannFitter::STRiemannFitter()
{
  fODRFitter = new ODRFitter();
}

Bool_t 
STRiemannFitter::FitData(std::vector<STHit*> *hitArray,
                         Double_t &xCenter, 
                         Double_t &zCenter, 
                         Double_t &radius,
                         Double_t &rms,
                         Double_t &rmsP)
{
  if (hitArray -> size() < 3)
    return kFALSE;

  fODRFitter -> Reset();

  Double_t xMean = 0;
  Double_t yMean = 0;
  Double_t xCov = 0;
  Double_t yCov = 0;
  Double_t weightSum = 0;

  for (auto hit : *hitArray)
  {
    Double_t x = hit -> GetPosition().X();
    Double_t y = hit -> GetPosition().Z();
    Double_t w = hit -> GetCharge();

    xMean = (weightSum / (weightSum + w)) * xMean + (w * x / (weightSum + w));
    yMean = (weightSum / (weightSum + w)) * yMean + (w * y / (weightSum + w));

    if (weightSum != 0) {
      xCov = (weightSum / (weightSum + w)) * xCov + (w * (x - xMean) * (x - xMean) / weightSum);
      yCov = (weightSum / (weightSum + w)) * yCov + (w * (y - yMean) * (y - yMean) / weightSum);
    }

    weightSum += w;
  }

  Double_t RSR = 2 * sqrt(xCov + yCov);

  Double_t xMapMean  = 0;
  Double_t yMapMean  = 0;
  Double_t zMapMean  = 0;

  for (auto hit : *hitArray)
  {
    Double_t x = hit -> GetPosition().X() - xMean;
    Double_t y = hit -> GetPosition().Z() - yMean;
    Double_t w = hit -> GetCharge();

    Double_t rEff = sqrt(x*x + y*y) / (2*RSR);
    Double_t denominator = 1 + rEff*rEff;

    Double_t xMap = x / denominator;
    Double_t yMap = y / denominator;
    Double_t zMap = 2 * RSR * rEff * rEff / denominator;

    xMapMean += w * xMap;
    yMapMean += w * yMap;
    zMapMean += w * zMap;
  }

  xMapMean = xMapMean / weightSum;
  yMapMean = yMapMean / weightSum;
  zMapMean = zMapMean / weightSum;

  fODRFitter -> SetCentroid(xMapMean, yMapMean, zMapMean);
  TVector3 mapMean(xMapMean, yMapMean, zMapMean);

  for (auto hit : *hitArray)
  {
    Double_t x = hit -> GetPosition().X() - xMean;
    Double_t y = hit -> GetPosition().Z() - yMean;
    Double_t w = hit -> GetCharge();

    Double_t rEff = sqrt(x*x + y*y) / (2*RSR);
    Double_t denominator = 1 + rEff*rEff;

    Double_t xMap = x / denominator;
    Double_t yMap = y / denominator;
    Double_t zMap = 2 * RSR * rEff * rEff / denominator;

    fODRFitter -> AddPoint(xMap, yMap, zMap, w);
  }

  /*
   * uOnPlane and vOnPlane are vectors on the plane.
   * nToPlane is vector normal to the plane.
   * All 3 vectors are orthogonal.
  */

  fODRFitter -> Solve();
  fODRFitter -> ChooseEigenValue(0); TVector3 uOnPlane = fODRFitter -> GetDirection();
  fODRFitter -> ChooseEigenValue(1); TVector3 vOnPlane = fODRFitter -> GetDirection();
  fODRFitter -> ChooseEigenValue(2); TVector3 nToPlane = fODRFitter -> GetDirection();

  /* XXX 
   * CAREFUL WHEN YOU CHANGE THE CODE!!!
   * Especially when you change the riemann sphere position on the plane.
   *
   * Following condition select out perfect line fit.
   * This is possible because "if data fits in perfect line", 
   * the point (xMean, yMean) lie on the fit line 
   * which is where riemann sphere is sitting on.
  */

  if (nToPlane.X() * nToPlane.X() + nToPlane.Y() * nToPlane.Y() == 1) {
    xCenter = nToPlane.Y();
    zCenter = nToPlane.X();
    radius = 0;
    rms = 0;
    rmsP = 0;
    return kTRUE;
  }

  rmsP = fODRFitter -> GetRMSPlane();

  /*
   *  RSC : Riemann Sphere Center
   *  RCC : Riemann Circle Center
   *  RCR : Riemann Circle Radius
   *
   * tRCC : Normal vector line constant for Riemann sphere center
   * CtoS : Distance between RSC and RCC.
  */

  TVector3  RSC = TVector3(0, 0, RSR);
  Double_t tRCC = nToPlane.Dot(mapMean - RSC) / nToPlane.Mag2();
  TVector3  RCC = tRCC * nToPlane + RSC;
  Double_t CtoS = (RCC - RSC).Mag();
  Double_t  RCR = sqrt(RSR*RSR - CtoS*CtoS);

  Double_t uConst = uOnPlane.Z() / sqrt(uOnPlane.Z()*uOnPlane.Z() + vOnPlane.Z()*vOnPlane.Z());
  Double_t vConst = sqrt(1-uConst*uConst);

  Double_t ref1 = uConst*uOnPlane.Z() + vConst*vOnPlane.Z();
  Double_t ref2 = uConst*uOnPlane.Z() - vConst*vOnPlane.Z();

  if (ref1 < 0) ref1 = -ref1;
  if (ref2 < 0) ref2 = -ref2;
  if (ref1 < ref2) vConst = -vConst;

  TVector3 toLouu = uConst * uOnPlane + vConst * vOnPlane;
  TVector3 louu = RCC + toLouu*RCR;
  TVector3 high = RCC - toLouu*RCR;

  TVector3 louuInvMap(louu.X()/(1-louu.Z()/(2*RSR)), louu.Y()/(1-louu.Z()/(2*RSR)), 0);
  TVector3 highInvMap(high.X()/(1-high.Z()/(2*RSR)), high.Y()/(1-high.Z()/(2*RSR)), 0);

  // Fit Circle Center
  TVector3 FCC = 0.5 * (louuInvMap + highInvMap);

  xCenter = FCC.X() + xMean;
  zCenter = FCC.Y() + yMean;
  radius = 0.5 * (louuInvMap - highInvMap).Mag();

  Double_t S = 0;
  for (auto hit : *hitArray)
  {
    TVector3 position = hit -> GetPosition();
    Double_t d = radius - sqrt((position.X() - xCenter)*(position.X() - xCenter) 
                             + (position.Z() - zCenter)*(position.Z() - zCenter));
    S += hit -> GetCharge() * d * d;
  }

  rms = sqrt(S / (weightSum * (1 - 3/hitArray -> size())));

  return kTRUE;
}

Bool_t 
STRiemannFitter::Fit(std::vector<TVector3> *data,
                     Double_t &xCenter, 
                     Double_t &yCenter, 
                     Double_t &radius,
                     Double_t &rms,
                     Double_t &rmsP)
{
  if (data -> size() < 3)
    return kFALSE;

  fODRFitter -> Reset();

  Double_t xMean = 0;
  Double_t yMean = 0;
  Double_t xCov = 0;
  Double_t yCov = 0;
  Double_t weightSum = 0;

  for (auto point : *data)
  {
    Double_t x = point.X();
    Double_t y = point.Y();
    Double_t w = point.Z();

    xMean = (weightSum / (weightSum + w)) * xMean + (w * x / (weightSum + w));
    yMean = (weightSum / (weightSum + w)) * yMean + (w * y / (weightSum + w));

    if (weightSum != 0) {
      xCov = (weightSum / (weightSum + w)) * xCov + (w * (x - xMean) * (x - xMean) / weightSum);
      yCov = (weightSum / (weightSum + w)) * yCov + (w * (y - yMean) * (y - yMean) / weightSum);
    }

    weightSum += w;
  }

  Double_t RSR = 2*sqrt(xCov + yCov);

  Double_t xMapMean  = 0;
  Double_t yMapMean  = 0;
  Double_t zMapMean  = 0;

  for (auto point : *data)
  {
    Double_t x = point.X() - xMean;
    Double_t y = point.Y() - yMean;
    Double_t w = point.Z();

    Double_t rEff = sqrt(x*x + y*y) / (2*RSR);
    Double_t denominator = 1 + rEff*rEff;

    Double_t xMap = x / denominator;
    Double_t yMap = y / denominator;
    Double_t zMap = 2 * RSR * rEff * rEff / denominator;

    xMapMean += w * xMap;
    yMapMean += w * yMap;
    zMapMean += w * zMap;
  }

  xMapMean = xMapMean / weightSum;
  yMapMean = yMapMean / weightSum;
  zMapMean = zMapMean / weightSum;

  fODRFitter -> SetCentroid(xMapMean, yMapMean, zMapMean);
  TVector3 mapMean(xMapMean, yMapMean, zMapMean);

  for (auto point : *data)
  {
    Double_t x = point.X() - xMean;
    Double_t y = point.Y() - yMean;
    Double_t w = point.Z();

    Double_t rEff = sqrt(x*x + y*y) / (2*RSR);
    Double_t denominator = 1 + rEff*rEff;

    Double_t xMap = x / denominator;
    Double_t yMap = y / denominator;
    Double_t zMap = 2 * RSR * rEff * rEff / denominator;

    fODRFitter -> AddPoint(xMap, yMap, zMap, w);
  }

  /*
   * uOnPlane and vOnPlane are vectors on the plane.
   * nToPlane is vector normal to the plane.
   * All 3 vectors are orthogonal.
  */

  fODRFitter -> Solve();
  fODRFitter -> ChooseEigenValue(0); TVector3 uOnPlane = fODRFitter -> GetDirection();
  fODRFitter -> ChooseEigenValue(1); TVector3 vOnPlane = fODRFitter -> GetDirection();
  fODRFitter -> ChooseEigenValue(2); TVector3 nToPlane = fODRFitter -> GetDirection();

  /* XXX 
   * CAREFUL WHEN YOU CHANGE THE CODE!!!
   * Especially when you change the riemann sphere position on the plane.
   *
   * Following condition select out perfect line fit.
   * This is possible because "if data fits in perfect line", 
   * the point (xMean, yMean) lie on the fit line 
   * which is where riemann sphere is sitting on.
  */

  if (nToPlane.X() * nToPlane.X() + nToPlane.Y() * nToPlane.Y() == 1) {
    xCenter = nToPlane.Y();
    yCenter = nToPlane.X();
    radius = 0;
    rms = 0;
    rmsP = 0;
    return kTRUE;
  }

  rmsP = fODRFitter -> GetRMSPlane();

  /*
   *  RSC : Riemann Sphere Center
   *  RCC : Riemann Circle Center
   *  RCR : Riemann Circle Radius
   *
   * tRCC : Normal vector line constant for Riemann sphere center
   * CtoS : Distance between RSC and RCC.
  */

  TVector3  RSC = TVector3(0, 0, RSR);
  Double_t tRCC = nToPlane.Dot(mapMean - RSC) / nToPlane.Mag2();
  TVector3  RCC = tRCC * nToPlane + RSC;
  Double_t CtoS = (RCC - RSC).Mag();
  Double_t  RCR = sqrt(RSR*RSR - CtoS*CtoS);

  Double_t uConst = uOnPlane.Z() / sqrt(uOnPlane.Z()*uOnPlane.Z() + vOnPlane.Z()*vOnPlane.Z());
  Double_t vConst = sqrt(1-uConst*uConst);

  Double_t ref1 = uConst*uOnPlane.Z() + vConst*vOnPlane.Z();
  Double_t ref2 = uConst*uOnPlane.Z() - vConst*vOnPlane.Z();

  if (ref1 < 0) ref1 = -ref1;
  if (ref2 < 0) ref2 = -ref2;
  if (ref1 < ref2) vConst = -vConst;

  TVector3 toLouu = uConst * uOnPlane + vConst * vOnPlane;
  TVector3 louu = RCC + toLouu*RCR;
  TVector3 high = RCC - toLouu*RCR;

  TVector3 louuInvMap(louu.X()/(1-louu.Z()/(2*RSR)), louu.Y()/(1-louu.Z()/(2*RSR)), 0);
  TVector3 highInvMap(high.X()/(1-high.Z()/(2*RSR)), high.Y()/(1-high.Z()/(2*RSR)), 0);

  // Fit Circle Center
  TVector3 FCC = 0.5 * (louuInvMap + highInvMap);

  xCenter = FCC.X() + xMean;
  yCenter = FCC.Y() + yMean;
  radius = 0.5 * (louuInvMap - highInvMap).Mag();

  Double_t S = 0;
  for (auto point : *data)
  {
    Double_t d = radius - sqrt((point.X() - xCenter)*(point.X() - xCenter) 
                             + (point.Y() - yCenter)*(point.Y() - yCenter));
    S += point.Z() * d * d;
  }

  rms = sqrt(S / (weightSum * (1 - 3/data -> size())));

  return kTRUE;
}
