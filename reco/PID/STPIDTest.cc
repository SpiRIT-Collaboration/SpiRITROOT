#include "STPIDTest.hh"
#include "TMath.h"

#include "float.h"
#include <iostream>
using namespace std;

ClassImp(STPIDTest)

STPIDTest::STPIDTest()
{
  if (STPID::kPion < NUMSTPID) {
    fdEdxFunction[STPID::kPion] = new TF1("dEdxFunctionPion",this,&STPIDTest::FitdEdxPion,40,490,1);
    fCoef[STPID::kPion] = 0.3;
  }

  if (STPID::kProton < NUMSTPID) {
    fdEdxFunction[STPID::kProton] = new TF1("dEdxFunctionProton",this,&STPIDTest::FitdEdxProton,60,2500,1);
    fCoef[STPID::kProton] = 0.3;
  }

  if (STPID::kDeuteron < NUMSTPID) {
    fdEdxFunction[STPID::kDeuteron] = new TF1("dEdxFunctionDeuteron",this,&STPIDTest::FitdEdxDeuteron,60,2500,1);
    fCoef[STPID::kDeuteron] = 0.3;
  }

  if (STPID::kTriton < NUMSTPID) {
    fdEdxFunction[STPID::kTriton] = new TF1("dEdxFunctionTriton",this,&STPIDTest::FitdEdxTriton,60,2500,1);
    fCoef[STPID::kTriton] = 0.2;
  }

  if (STPID::k3He < NUMSTPID) {
    fdEdxFunction[STPID::k3He] = new TF1("dEdxFunction3He",this,&STPIDTest::FitdEdx3He,60,2500,1);
    fCoef[STPID::k3He] = 0.15;
  }

  if (STPID::k4He < NUMSTPID) {
    fdEdxFunction[STPID::k4He] = new TF1("dEdxFunction4He",this,&STPIDTest::FitdEdx4He,60,2500,1);
    fCoef[STPID::k4He] = 0.15;
  }

  fGausFunction = new TF1("fGausFunction","gaus",0,800);
}

Double_t STPIDTest::GetProbability(STPID::PID stpid, Double_t p, Double_t dedx)
{
  Double_t prob = 0;
  if (stpid == STPID::kPion && p > 500)
    prob = 0;
  else {
    auto mu = fdEdxFunction[stpid] -> Eval(p);
    auto sigma = fCoef[stpid] * fdEdxFunction[stpid] -> Eval(p);

    fGausFunction -> SetParameters(1,mu,sigma);
    prob = fGausFunction -> Eval(dedx);
  }

  if (stpid == STPID::kProton && prob == 0)
    prob = DBL_MIN;

  return prob;
}

TF1 *STPIDTest::GetdEdxFunction(STPID::PID stpid) { return fdEdxFunction[stpid]; } 
void STPIDTest::SetCoef(STPID::PID stpid, Double_t coef) { fCoef[stpid] = coef; }

Double_t STPIDTest::MeVToADC(Double_t val) { return (val-fADCToMeVOffset)/fADCToMeVFactor; }
Double_t STPIDTest::ADCToMeV(Double_t val) { return val*fADCToMeVFactor+fADCToMeVOffset; }

void STPIDTest::SetADCToMeVFactor(Double_t val) { fADCToMeVFactor = val; }
void STPIDTest::SetADCToMeVOffSet(Double_t val) { fADCToMeVOffset = val; }
Double_t STPIDTest::GetADCToMeVFactor() { return fADCToMeVFactor; }
Double_t STPIDTest::GetADCToMeVOffSet() { return fADCToMeVFactor; }

Double_t STPIDTest::FitdEdxPion(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t dedx = (xv<260)*(2.02484e-05 + 0.000460201/TMath::Log(xv) + 1.7545/(xv*xv)) + (xv>=260)*(0.00018498 - 3.10648e-07*xv + 3.52378e-10*xv*xv);
  return MeVToADC(dedx);
}

Double_t STPIDTest::FitdEdxProton(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t dedx = (xv<490)*(0.000170506 -56764.6/(xv*xv*xv*xv) + 76.1793/(xv*xv)) +
    (xv>=490)*(0.00218582 -6.96452e-06*xv + 1.01583e-08*xv*xv -7.91095e-12*xv*xv*xv +
	       3.41111e-15*xv*xv*xv*xv -7.67179e-19*xv*xv*xv*xv*xv + 7.01878e-23*xv*xv*xv*xv*xv*xv);
  return MeVToADC(dedx);
}

Double_t STPIDTest::FitdEdxDeuteron(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t dedx = (xv<490)*(0.000375736 + 271.322/(xv*xv) +  3.43658e-08*TMath::Log(xv)) +
    (xv>=490 && xv< 1000)*(0.0176734 -8.42645e-05*xv + 1.62116e-07*xv*xv -1.40286e-10*xv*xv*xv + 4.52213e-14*xv*xv*xv*xv)+
    (xv>=1000)*(0.00250854 -4.23881e-06*xv + 3.05739e-09*xv*xv -1.01574e-12*xv*xv*xv + 1.28167e-16*xv*xv*xv*xv);
  return MeVToADC(dedx);
}

Double_t STPIDTest::FitdEdxTriton(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t dedx = (xv<700)*(0.000373171 + 609.022/(xv*xv) +  2.4014e-06*TMath::Log(xv)) +
    (xv>=700 && xv< 1500)*(0.0173069 -5.47368e-05*xv + 6.99956e-08*xv*xv -4.02975e-11*xv*xv*xv + 8.64625e-15*xv*xv*xv*xv) +
    (xv>=1500)*(0.00927863 -1.61959e-05*xv + 1.1119e-08*xv*xv -3.42519e-12*xv*xv*xv + 3.95728e-16*xv*xv*xv*xv);
  return MeVToADC(dedx);
}

Double_t STPIDTest::FitdEdx3He(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t dedx = (xv<550)*(0.00176161 + 2411.02/(xv*xv) +  7.83326e-06*TMath::Log(xv)) +
    (xv>=550 && xv< 1500)*(0.0647784 -0.000200808*xv + 2.53075e-07*xv*xv -1.43952e-10*xv*xv*xv + 3.05516e-14*xv*xv*xv*xv) +
    (xv>=1500)*(0.0371145 -6.47836e-05*xv + 4.4476e-08*xv*xv -1.37008e-11*xv*xv*xv + 1.58291e-15*xv*xv*xv*xv);
  return MeVToADC(dedx);
}

Double_t STPIDTest::FitdEdx4He(Double_t *x, Double_t *par) {
  Double_t xv = x[0];
  Double_t dedx = (xv<800)*(0.00162818 + 4297.73/(xv*xv) +  1.8015e-05*TMath::Log(xv)) +
    (xv>=800 && xv< 1800)*(0.0680528 -0.00016068*xv + 1.53621e-07*xv*xv -6.61668e-11*xv*xv*xv + 1.06244e-14*xv*xv*xv*xv) +
    (xv>=1800)*(0.00558405 + 7.55158e-06*xv -1.23558e-08*xv*xv + 5.24654e-12*xv*xv*xv -7.15741e-16*xv*xv*xv*xv);
  return MeVToADC(dedx);
}
