// SpiRITROOT classes
#include "STClusterizer.hh"

// FairRoot classes
#include "FairRuntimeDb.h"
#include "FairRun.h"

// STL
#include <algorithm>

STClusterizer::STClusterizer()
{
  fLogger = FairLogger::GetLogger();

  FairRun *run = FairRun::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  STDigiPar* par = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!par)
    fLogger -> Fatal(MESSAGE_ORIGIN, "STDigiPar not found!!");

  fPrimaryVertex = TVector3(0., -213.3, -35.2);

  fPadPlaneX      = par -> GetPadPlaneX();
  fPadSizeX       = par -> GetPadSizeX();
  fPadSizeZ       = par -> GetPadSizeZ();
  fPadRows        = par -> GetPadRows();
  fPadLayers      = par -> GetPadLayers();
  fNumTbs         = par -> GetWindowNumTbs();
  fTBTime         = par -> GetTBTime();
  fDriftVelocity  = par -> GetDriftVelocity();
  fMaxDriftLength = par -> GetDriftLength();

  fYTb = fTBTime * fDriftVelocity/100.;

  fXLowCut  = -424.;
  fXHighCut =  424.;

  SetProximityCutInUnit(1.5, 2.5, 0.5);
  SetSigmaCutInUnit(2.0, 2.0, 0.8);
}

STClusterizer::~STClusterizer()
{
}

void STClusterizer::SetPrimaryVertex(TVector3 vertex) { fPrimaryVertex = vertex; }
void STClusterizer::SetProximityCut(Double_t x, Double_t y, Double_t z)
{
  fXCut = x;
  fYCut = y;
  fZCut = z;
}
void STClusterizer::SetProximityCutInUnit(Double_t xw, Double_t yw, Double_t zw)
{
  fXCut = fPadSizeX * xw;
  fYCut = fYTb * yw;
  fZCut = fPadSizeZ * zw;
}
void STClusterizer::SetSigmaCut(Double_t x, Double_t y, Double_t z)
{
  fSigmaXCut = x;
  fSigmaYCut = y;
  fSigmaZCut = z;
}
void STClusterizer::SetSigmaCutInUnit(Double_t xw, Double_t yw, Double_t zw)
{
  fSigmaXCut = fPadSizeX * xw;
  fSigmaYCut = fYTb * yw;
  fSigmaZCut = fPadSizeZ * zw;
}
void STClusterizer::SetEdgeCut(Double_t low, Double_t high)
{
  fXLowCut = low;
  fXHighCut = high;
}
