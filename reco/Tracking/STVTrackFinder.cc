/**
 * @author JungWoo Lee
 */

#include "STVTrackFinder.hh"
#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "STDigiPar.hh"
#include "STParReader.hh"

ClassImp(STVTrackFinder)

STVTrackFinder::STVTrackFinder()
{
  fLogger = FairLogger::GetLogger();
  FairRun *run = FairRun::Instance();
  FairRuntimeDb *db = run -> GetRuntimeDb();
  STDigiPar *par = (STDigiPar *) db -> getContainer("STDigiPar");

  fNumTbs = par -> GetNumTbs();
  Double_t tbTime = par -> GetTBTime();
  Double_t driftVel = par -> GetDriftVelocity();

  fXUnit = par -> GetPadSizeX();
  fYUnit = tbTime * driftVel / 100.;
  fZUnit = par -> GetPadSizeZ();

  TString parName = par -> GetTrackingParFileName();
  STParReader *fTrackingPar = new STParReader(parName);

  fNumHitsTrackCut   = fTrackingPar -> GetIntPar("NumHitsTrackCut");
  fNumHitsFit        = fTrackingPar -> GetIntPar("NumHitsFit");
  fNumHitsCompare    = fTrackingPar -> GetIntPar("NumHitsCompare");
  fNumHitsCompareMax = fTrackingPar -> GetIntPar("NumHitsCompareMax");
  fProxLineCut       = fTrackingPar -> GetDoublePar("ProxLineCut");
  fProxPlaneCut      = fTrackingPar -> GetDoublePar("ProxPlaneCut");
  fProxRCut          = fTrackingPar -> GetDoublePar("ProxRCut");
  fRMSLineCut        = fTrackingPar -> GetDoublePar("RMSLineCut");
  fRMSPlaneCut       = fTrackingPar -> GetDoublePar("RMSPlaneCut");
  fDirectionDotCut   = fTrackingPar -> GetDoublePar("DirectionDotCut");
  fNormalDotCut      = fTrackingPar -> GetDoublePar("NormalDotCut");
  fRadialCut         = fTrackingPar -> GetDoublePar("RadialCut");
  fNumHitsVanishCut  = fTrackingPar -> GetDoublePar("NumHitsVanishCut");

  Double_t xConst    = fTrackingPar -> GetDoublePar("XConst");
  Double_t yConst    = fTrackingPar -> GetDoublePar("YConst");
  Double_t zConst    = fTrackingPar -> GetDoublePar("ZConst");

  fProxXCut = xConst * fXUnit;
  fProxYCut = yConst * fYUnit;
  fProxZCut = zConst * fZUnit;

  fRMSTrackCut = sqrt(fRMSPlaneCut*fRMSPlaneCut + fRMSLineCut*fRMSLineCut);

  fVertex = TVector3(fTrackingPar -> GetDoublePar("XVertex"),
                     fTrackingPar -> GetDoublePar("YVertex"),
                     fTrackingPar -> GetDoublePar("ZVertex"));

  fNumHitsAtHead = fTrackingPar -> GetIntPar("NumHitsAtHead");
}
