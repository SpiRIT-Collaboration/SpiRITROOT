/**
 * @brief Circle Fitter
 *
 * @author JungWoo Lee
 */

#include "STCircleFitter.hh"

ClassImp(STCircleFitter)

Bool_t 
STCircleFitter::FitData(std::vector<STHit*> *hitArray,
                        Double_t &xCenter, 
                        Double_t &zCenter, 
                        Double_t &radius,
                        Double_t &rms)
{
  Double_t expectationX = 0;
  Double_t expectationZ = 0;
  Double_t chargeSum = 0;

  for (auto hit : *hitArray)
  {
    Double_t charge = hit -> GetCharge();
    TVector3 position = hit -> GetPosition();

    expectationX += charge * position.X();
    expectationZ += charge * position.Z();

    chargeSum += charge;
  }

  expectationX *= 1./chargeSum;
  expectationZ *= 1./chargeSum;

  Double_t expectationU3  = 0;
  Double_t expectationU2  = 0;
  Double_t expectationV3  = 0;
  Double_t expectationV2  = 0;
  Double_t expectationUV  = 0;
  Double_t expectationU2V = 0;
  Double_t expectationUV2 = 0;

  for (auto hit : *hitArray)
  {
    Double_t charge = hit -> GetCharge();
    TVector3 position = hit -> GetPosition();

    Double_t u = position.X() - expectationX;
    Double_t v = position.Z() - expectationZ;

    expectationU3  += charge * u * u * u;
    expectationU2  += charge * u * u;
    expectationV3  += charge * v * v * v;
    expectationV2  += charge * v * v;
    expectationUV  += charge * u * v;
    expectationU2V += charge * u * u * v;
    expectationUV2 += charge * u * v * v;
  }

  expectationU3  *= 1./chargeSum;
  expectationU2  *= 1./chargeSum;
  expectationV3  *= 1./chargeSum;
  expectationV2  *= 1./chargeSum;
  expectationUV  *= 1./chargeSum;
  expectationU2V *= 1./chargeSum;
  expectationUV2 *= 1./chargeSum;

  Double_t admbc = expectationU2 *expectationV2 - expectationUV * expectationUV;

  if (admbc == 0)
    return kFALSE;

  Double_t m = expectationU3 + expectationUV2;
  Double_t n = expectationU2V + expectationV3;

  Double_t uC = (expectationV2 * m - expectationUV * n) / admbc / 2.;
  Double_t vC = (expectationU2 * n - expectationUV * m) / admbc / 2.;

  xCenter = uC + expectationX;
  zCenter = vC + expectationZ;

  radius = sqrt(uC * uC + vC * vC + expectationU2 + expectationV2);

  Double_t S = 0;
  for (auto hit : *hitArray)
  {
    TVector3 pos = hit -> GetPosition();
    Double_t d = radius - sqrt((pos.X() - xCenter)*(pos.X() - xCenter) 
                             + (pos.Z() - zCenter)*(pos.Z() - zCenter));
    S += hit -> GetCharge() * d * d;
  }

  rms = sqrt(S / (chargeSum * (1 - 3/hitArray -> size())));


  return kTRUE;
}

Bool_t 
STCircleFitter::Fit(std::vector<TVector3> *data,
                    Double_t &xCenter, 
                    Double_t &yCenter, 
                    Double_t &radius,
                    Double_t &rms)
{
  Double_t expectationX = 0;
  Double_t expectationY = 0;
  Double_t weightSum = 0;

  for (auto point : *data)
  {
    Double_t weight = point.Z();

    expectationX += weight * point.X();
    expectationY += weight * point.Y();

    weightSum += weight;
  }

  expectationX *= 1./weightSum;
  expectationY *= 1./weightSum;

  Double_t expectationU3  = 0;
  Double_t expectationU2  = 0;
  Double_t expectationV3  = 0;
  Double_t expectationV2  = 0;
  Double_t expectationUV  = 0;
  Double_t expectationU2V = 0;
  Double_t expectationUV2 = 0;

  for (auto point : *data)
  {
    Double_t weight = point.Z();

    Double_t u = point.X() - expectationX;
    Double_t v = point.Y() - expectationY;

    expectationU3  += weight * u * u * u;
    expectationU2  += weight * u * u;
    expectationV3  += weight * v * v * v;
    expectationV2  += weight * v * v;
    expectationUV  += weight * u * v;
    expectationU2V += weight * u * u * v;
    expectationUV2 += weight * u * v * v;
  }

  expectationU3  *= 1./weightSum;
  expectationU2  *= 1./weightSum;
  expectationV3  *= 1./weightSum;
  expectationV2  *= 1./weightSum;
  expectationUV  *= 1./weightSum;
  expectationU2V *= 1./weightSum;
  expectationUV2 *= 1./weightSum;

  Double_t admbc = expectationU2 *expectationV2 - expectationUV * expectationUV;

  if (admbc == 0)
    return kFALSE;

  Double_t m = expectationU3 + expectationUV2;
  Double_t n = expectationU2V + expectationV3;

  Double_t uC = (expectationV2 * m - expectationUV * n) / admbc / 2.;
  Double_t vC = (expectationU2 * n - expectationUV * m) / admbc / 2.;

  xCenter = uC + expectationX;
  yCenter = vC + expectationY;

  radius = sqrt(uC * uC + vC * vC + expectationU2 + expectationV2);

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
