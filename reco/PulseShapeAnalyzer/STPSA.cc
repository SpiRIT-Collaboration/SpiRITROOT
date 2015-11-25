//-----------------------------------------------------------
// Description:
//   Simple version of analyzing pulse shape of raw signal.
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//   JungWoo Lee     korea University
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STPSA.hh"

// FairRoot classes
#include "FairRuntimeDb.h"
#include "FairRun.h"

// STL
#include <algorithm>

using std::max_element;
using std::min_element;
using std::distance;

ClassImp(STPSA)

STPSA::STPSA()
{
  fLogger = FairLogger::GetLogger();

  FairRun *run = FairRun::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "STDigiPar not found!!");

  fPadPlaneX = fPar -> GetPadPlaneX();
  fPadSizeX = fPar -> GetPadSizeX();
  fPadSizeZ = fPar -> GetPadSizeZ();
  fPadRows = fPar -> GetPadRows();
  fPadLayers = fPar -> GetPadLayers();

  fNumTbs = fPar -> GetNumTbs();
  fNumTbsWindow = fPar -> GetWindowNumTbs();
  fTBTime = fPar -> GetTBTime();
  fDriftVelocity = fPar -> GetDriftVelocity();
  fMaxDriftLength = fPar -> GetDriftLength();

  fThreshold = -1;
}

STPSA::~STPSA()
{
}

void
STPSA::LSLFit(Int_t numPoints, Double_t *x, Double_t *y, Double_t &constant, Double_t &slope, Double_t &chi2)
{
  Double_t sumXY = 0, sumX = 0, sumY = 0, sumX2 = 0;
  for (Int_t iPoint = 0; iPoint < numPoints; iPoint++) {
    sumXY += x[iPoint]*y[iPoint];
    sumX += x[iPoint];
    sumY += y[iPoint];
    sumX2 += x[iPoint]*x[iPoint];
  }

  slope = (numPoints*sumXY - sumX*sumY)/(numPoints*sumX2 - sumX*sumX);
  constant = (sumX2*sumY - sumX*sumXY)/(numPoints*sumX2 - sumX*sumX);

  chi2 = 0;
  for (Int_t iPoint = 0; iPoint < numPoints; iPoint++)
    chi2 += pow(x[iPoint]*slope + constant - y[iPoint], 2);
}

void STPSA::SetThreshold(Double_t threshold) { fThreshold = threshold; }
void STPSA::SetLayerCut(Int_t layerCut)      { fLayerCut = layerCut; }

Double_t
STPSA::CalculateX(Double_t row)
{
  return (row + 0.5)*fPadSizeX - fPadPlaneX/2.;
}

Double_t
STPSA::CalculateY(Double_t peakIdx)
{
  return -peakIdx*fTBTime*fDriftVelocity/100.;
}

Double_t
STPSA::CalculateZ(Double_t layer)
{
  return (layer + 0.5)*fPadSizeZ;
}
