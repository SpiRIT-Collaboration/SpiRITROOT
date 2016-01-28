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

  TString parName = par -> GetLTParFileName();
  STParReader *fLTPar = new STParReader(parName);

  fNumHitsTrackCut   = fLTPar -> GetIntPar("NumHitsTrackCut");
  fNumHitsFit        = fLTPar -> GetIntPar("NumHitsFit");
  fNumHitsCompare    = fLTPar -> GetIntPar("NumHitsCompare");
  fNumHitsCompareMax = fLTPar -> GetIntPar("NumHitsCompareMax");
  fProxLineCut       = fLTPar -> GetDoublePar("ProxLineCut");
  fProxPlaneCut      = fLTPar -> GetDoublePar("ProxPlaneCut");
  fProxRCut          = fLTPar -> GetDoublePar("ProxRCut");
  fRMSLineCut        = fLTPar -> GetDoublePar("RMSLineCut");
  fRMSPlaneCut       = fLTPar -> GetDoublePar("RMSPlaneCut");
  fDirectionDotCut   = fLTPar -> GetDoublePar("DirectionDotCut");
  fNormalDotCut      = fLTPar -> GetDoublePar("NormalDotCut");
  fPerpYCut          = fLTPar -> GetDoublePar("PerpYCut");
  fNumHitsVanishCut  = fLTPar -> GetDoublePar("NumHitsVanishCut");

  Double_t xConst    = fLTPar -> GetDoublePar("XConst");
  Double_t yConst    = fLTPar -> GetDoublePar("YConst");
  Double_t zConst    = fLTPar -> GetDoublePar("ZConst");

  fProxXCut = xConst * fXUnit;
  fProxYCut = yConst * fYUnit;
  fProxZCut = zConst * fZUnit;
  fRMSTrackCut = TMath::Sqrt(fRMSPlaneCut*fRMSPlaneCut + fRMSLineCut*fRMSLineCut);
}
