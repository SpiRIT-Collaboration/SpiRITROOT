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
#include "STParReader.hh"

// FairRoot classes
#include "FairRuntimeDb.h"
#include "FairRun.h"

// STL
#include <algorithm>

using std::max_element;
using std::min_element;
using std::distance;

#include <fstream>
#include <iostream>

using namespace std;

ClassImp(STPSA)

STPSA::STPSA()
{
  fLogger = FairLogger::GetLogger();

  FairRun *run = FairRun::Instance();

  TString uaName;
  TString agetName;

  if (run)
  {
    FairRuntimeDb *db = run -> GetRuntimeDb();
    if (!db)
      fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

    fPar = (STDigiPar *) db -> getContainer("STDigiPar");
    if (!fPar)
      fLogger -> Fatal(MESSAGE_ORIGIN, "STDigiPar not found!!");

    fPadPlaneX      = fPar -> GetPadPlaneX();
    fPadSizeX       = fPar -> GetPadSizeX();
    fPadSizeZ       = fPar -> GetPadSizeZ();
    fPadRows        = fPar -> GetPadRows();
    fPadLayers      = fPar -> GetPadLayers();
    fNumTbs         = fPar -> GetNumTbs();
    fWindowNumTbs   = fPar -> GetWindowNumTbs();
    fWindowStartTb  = fPar -> GetWindowStartTb();
    fTBTime         = fPar -> GetTBTime();
    fDriftVelocity  = fPar -> GetDriftVelocity();
    fMaxDriftLength = fPar -> GetDriftLength();

    uaName   = fPar -> GetUAMapFileName();
    agetName = fPar -> GetAGETMapFileName();

    fThreshold = -1;
  }

  else
  {
    TString workDir = gSystem -> Getenv("VMCWORKDIR");
    TString parameterFile = workDir + "/parameters/ST.parameters.par";
    STParReader *fParReader = new STParReader(parameterFile);

    fPadPlaneX      = fParReader -> GetDoublePar("PadPlaneX");
    fPadSizeX       = fParReader -> GetDoublePar("PadSizeX");
    fPadSizeZ       = fParReader -> GetDoublePar("PadSizeZ");
    fPadRows        = fParReader -> GetIntPar("PadRows");
    fPadLayers      = fParReader -> GetIntPar("PadLayers");
    fNumTbs         = fParReader -> GetIntPar("NumTbs");
    fWindowNumTbs   = fParReader -> GetIntPar("WindowNumTbs");
    fWindowStartTb  = fParReader -> GetIntPar("WindowStartTb");
    fDriftVelocity  = fParReader -> GetDoublePar("DriftVelocity");
    fMaxDriftLength = fParReader -> GetDoublePar("DriftLength");

    uaName   = fParReader -> GetFilePar(3);
    agetName = fParReader -> GetFilePar(2);

    Int_t samplingRate = fParReader -> GetIntPar("SamplingRate");
    switch (samplingRate) {
      case 25:  fTBTime = 40; break;
      case 50:  fTBTime = 20; break;
      case 100: fTBTime = 10; break;
      default:  fTBTime = -1; break;
    }

    fThreshold = 20;
  }

  fWindowEndTb = fWindowStartTb + fWindowNumTbs;
  if (fWindowEndTb > 512) 
    fWindowEndTb = 512;

  fTbToYConv = -fTBTime * fDriftVelocity / 100.;

  fPadMap = new STMap();
  fPadMap -> SetUAMap(uaName);
  fPadMap -> SetAGETMap(agetName);

  for (auto coboIdx = 0; coboIdx < 12; ++coboIdx)
    fTbOffsets[coboIdx] = 0.;

  for (auto layer = 0; layer < 112; ++layer)
    for (auto row = 0; row < 108; ++row)
      fYOffsets[layer][row] = 0.;

  fYPedestalOffset = 0;
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

void STPSA::SetThreshold(Double_t threshold) 
{ 
  fThreshold = threshold; 
  fThresholdOneTbStep = fThreshold/5.;

  if (fThresholdOneTbStep > 2) 
    fThresholdOneTbStep = 2;

  if (fThresholdOneTbStep < 1) 
    fThresholdOneTbStep = 1;
}
void STPSA::SetLayerCut(Int_t layerLowCut, Int_t layerHighCut)
{ 
  fLayerLowCut = layerLowCut; 
  fLayerHighCut = layerHighCut; 
}

Double_t
STPSA::CalculateX(Double_t row)
{
  return (row + 0.5)*fPadSizeX - fPadPlaneX/2.;
}

Double_t
STPSA::CalculateY(Double_t peakIdx)
{
  return peakIdx * fTbToYConv;
}

Double_t
STPSA::CalculateZ(Double_t layer)
{
  return (layer + 0.5)*fPadSizeZ;
}

void STPSA::SetWindowStartTb(Int_t value) { fWindowStartTb = value; }

void STPSA::SetTbOffsets(Double_t *tbOffsets) {
  for (auto coboIdx = 0; coboIdx < 12; ++coboIdx)
    fTbOffsets[coboIdx] = tbOffsets[coboIdx];
}

void STPSA::SetYOffsets(TString fileName)
{
  Int_t layer, row, n;
  Double_t amp, off, err;
  ifstream calfile(fileName);
  while (calfile >> layer >> row >> n >> amp >> off >> err) {
    if (abs(off) > 4 || err > 2 || n < 20)
      off = 0;
    fYOffsets[layer][row] = off;
//    cout << layer << " " << row << " " << off << endl;
  }
}

void STPSA::SetYPedestalOffset(Double_t offset)
{
  fYPedestalOffset = offset;
}
