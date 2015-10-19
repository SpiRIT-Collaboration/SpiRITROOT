/**
 * @author JungWoo Lee
 */

#include "STLinearTrackFinderAbstract.hh"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "STDigiPar.hh"

ClassImp(STLinearTrackFinderAbstract)

STLinearTrackFinderAbstract::STLinearTrackFinderAbstract()
{
  FairRun *run = FairRun::Instance();
  FairRuntimeDb *db = run -> GetRuntimeDb();
  STDigiPar *par = (STDigiPar *) db -> getContainer("STDigiPar");

  fNumTbs = par -> GetNumTbs();
  Double_t tbTime = par -> GetTBTime();
  Double_t driftVel = par -> GetDriftVelocity();

  fXUnit = par -> GetPadSizeX();
  fYUnit = tbTime * driftVel / 100.;
  fZUnit = par -> GetPadSizeZ();

  SetNumHitsCut(20, 5, 8, 10);
  SetProximityCutFactor(1.01, 2.0, 1.01);
  SetProximityRCut(20);
  SetRMSCut(18, 1);
  SetDotProductCut(0.85, 0.9);
}

void 
STLinearTrackFinderAbstract::SetNumHitsCut(
  Int_t numHitsCut,
  Int_t numHitsHHCompare,
  Int_t numHitsFitLine,
  Int_t numHitsFitPlane)
{
  fMinNumHitCut = numHitsCut;
  fNumHHCompare = numHitsHHCompare;
  fMinNumHitFitLine = numHitsFitLine;
  fMinNumHitFitPlane = numHitsFitPlane;
}

void 
STLinearTrackFinderAbstract::SetProximityCutFactor(
  Double_t xConst, 
  Double_t yConst,
  Double_t zConst)
{
  fProxXCut = xConst * fXUnit;
  fProxYCut = yConst * fYUnit;
  fProxZCut = zConst * fZUnit;
}
void
STLinearTrackFinderAbstract::SetProximityRCut(Double_t val)
{
  fProxRCut = val;
}

void 
STLinearTrackFinderAbstract::SetRMSCut(Double_t rmsLineCut, Double_t rmsPlaneCut)
{
  fRMSLineCut  = rmsLineCut;
  fRMSPlaneCut = rmsPlaneCut;
  fRMSTrackCut = TMath::Sqrt(fRMSPlaneCut*fRMSPlaneCut + fRMSLineCut*fRMSLineCut);
}

void 
STLinearTrackFinderAbstract::SetDotProductCut(
  Double_t directionDotCut, 
  Double_t normalDotCut)
{
  fDirectionDotCut = directionDotCut;
  fNormalDotCut = normalDotCut;
}
