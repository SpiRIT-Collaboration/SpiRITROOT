#ifndef STMassCalculator_h
#define STMassCalculator_h 1

#include "STBBFunction.hh"
#include "TFile.h"
#include "TGraph2D.h"
#include "TVector3.h"
#include "Math/RootFinder.h"
#include "Math/Functor.h"
#include "functional"
#include "FairLogger.h"
#include <ROOT/TSeq.hxx>

#include <ROOT/TSeq.hxx>

class STMassCalculator
{
public:
  STMassCalculator();
  ~STMassCalculator();

  void LoadCalibrationParameters(TString fileName, UInt_t beamA);
  void   SetTGraph2D(TGraph2D* g2Par0, TGraph2D* g2Par1);
  void   SetParameter(TString fileName);

  Double_t CalcMass(UInt_t fpid, Double_t z,TVector3 mom,Double_t dEdx);
  Double_t CalcMass(Double_t z,TVector3 mom,Double_t dEdx);

  Double_t CalibdEdx(UInt_t fpid, TVector3 mom,Double_t dEdx);
  Double_t CalibdEdx(TVector3 mom,Double_t dEdx);

private:
  void   SetTGraph2D(UInt_t fpid);

private:
  FairLogger  *fLogger;       //!   
  TFile    *fFile;
  TString  fGraphName;
  TGraph2D *g2PIDCalib2[2][2]; //[p or d][par[0] or par[1]]
  TGraph2D *g2PIDCalib[2];     //[p or d][par[0] or par[1]]
  Bool_t   isLoadParameter;
  ROOT::Math::RootFinder finder;
  Double_t *bbPar;
  STBBFunction bbfunc;
  std::function<double(double)> funcBB;
  std::function<double(double)> dfuncBB;
  ROOT::Math::GradFunctor1D *gradfunc1dBB;

};
#endif

