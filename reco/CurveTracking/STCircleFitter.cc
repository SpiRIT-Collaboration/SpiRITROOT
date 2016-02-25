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
                     Double_t &radius)
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

  return kTRUE;
}
