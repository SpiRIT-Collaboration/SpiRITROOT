//-----------------------------------------------------------
// Description:
//   Layer-based PSA
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STPSALayer.hh"
#include "STProcessManager.hh"

// STL
#include <cmath>
#include <cstdlib>

// ROOT classes
#include "TH2D.h"

ClassImp(STPSALayer)

STPSALayer::STPSALayer()
{

#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, "Start Initializing!");
#endif

  fPeakFinder = new TSpectrum();

  fNumSidePads = 2;
  fNumSideTbs = 2;
  fPeakStorageSize = 50;

  fMinPoints = 4;
  fPercPeakMin = 10;
  fPercPeakMax = 90;

  fNumPeaks = new Int_t*[fPadRows];
  fPeakTbs = new Int_t**[fPadRows];
  fPeakValues = new Double_t**[fPadRows];
  for (Int_t iRow = 0; iRow < fPadRows; iRow++) {
    fNumPeaks[iRow] = new Int_t[fPadLayers];
    fPeakTbs[iRow] = new Int_t*[fPadLayers];
    fPeakValues[iRow] = new Double_t*[fPadLayers];
    for (Int_t iLayer = 0; iLayer < fPadLayers; iLayer++) {
      fNumPeaks[iRow][iLayer] = 0;
      fPeakTbs[iRow][iLayer] = new Int_t[fPeakStorageSize];
      fPeakValues[iRow][iLayer] = new Double_t[fPeakStorageSize];
      for (Int_t iPeak = 0; iPeak < fPeakStorageSize; iPeak++) {
        fPeakTbs[iRow][iLayer][iPeak] = 0;
        fPeakValues[iRow][iLayer][iPeak] = 0;
      }
    }
  }

#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, "Initialize completed!");
#endif
}

STPSALayer::~STPSALayer()
{
}

void
STPSALayer::Analyze(STRawEvent *rawEvent, STEvent *event)
{
  PreAnalyze(rawEvent);

#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, "Start analyzing the evnet!");
#endif

  Int_t hitNum = 0;

  STPad pad = fPadArray -> at(0);
  Int_t totalPads = fPadArray -> size();

#ifndef DEBUG
  STProcessManager manager("STPSALayer", totalPads);
#endif

  whileStart:
  while (rawEvent -> GetNumPads()) {

#ifdef DEBUG
    fLogger -> Info(MESSAGE_ORIGIN, Form("Start with pad row: %d, layer: %d, rest pads: %d", pad.GetRow(), pad.GetLayer(), rawEvent -> GetNumPads()));
#endif

    Int_t row = pad.GetRow();
    Int_t layer = pad.GetLayer();

    Int_t numPeaks = fNumPeaks[row][layer];
    if (!numPeaks) {

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("No peak in pad row: %d, layer: %d!", pad.GetRow(), pad.GetLayer()));
#endif

      DeletePeakInfo(rawEvent, row, layer, 0);
      if (rawEvent -> GetNumPads())
        pad = fPadArray -> at(0);

      goto whileStart;
    }

    Int_t peakTb = fPeakTbs[row][layer][0];
    Double_t peakValue = fPeakValues[row][layer][0];

    if (peakValue < fThreshold) {

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("Peak isn't bigger than threshold: %f in pad row: %d, layer: %d!", peakValue, pad.GetRow(), pad.GetLayer()));
#endif

      DeletePeakInfo(rawEvent, row, layer, 0);

      goto whileStart;
    }

    if (!(row == 0)) {

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("Checking left pad row: %d, layer: %d!", row - 1, layer));
#endif

      Int_t leftNumPeaks = fNumPeaks[row - 1][layer];
      if (leftNumPeaks) {
        for (Int_t iPeak = 0; iPeak < leftNumPeaks; iPeak++) {
          Int_t leftPeakTb = fPeakTbs[row - 1][layer][iPeak];
          Double_t leftPeakValue = fPeakValues[row - 1][layer][iPeak];

          if (TMath::Abs(leftPeakTb - peakTb) < fNumSideTbs && peakValue < leftPeakValue) {

#ifdef DEBUG
            fLogger -> Info(MESSAGE_ORIGIN, Form("peakTb: %d, peakValue: %f, left tb: %d, peakValue: %f!", peakTb, peakValue, leftPeakTb, leftPeakValue));
#endif

            if (rawEvent -> GetNumPads()) {
              STPad *nextPad = rawEvent -> GetPad(row - 1, layer);
              pad = (!nextPad ? fPadArray -> at(0) : *nextPad);
            }

            goto whileStart;
          }
        }
      }
    }

    if (!(row + 1 == fPadRows)) {

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("Checking right pad row: %d, layer: %d!", row + 1, layer));
#endif

      Int_t rightNumPeaks = fNumPeaks[row + 1][layer];
      if (rightNumPeaks) {
        for (Int_t iPeak = 0; iPeak < rightNumPeaks; iPeak++) {
          Int_t rightPeakTb = fPeakTbs[row + 1][layer][iPeak];
          Double_t rightPeakValue = fPeakValues[row + 1][layer][iPeak];

          if (TMath::Abs(rightPeakTb - peakTb) < fNumSideTbs && peakValue < rightPeakValue) {

#ifdef DEBUG
            fLogger -> Info(MESSAGE_ORIGIN, Form("peakTb: %d, peakValue: %f, right tb: %d, peakValue: %f!", peakTb, peakValue, rightPeakTb, rightPeakValue));
#endif

            if (rawEvent -> GetNumPads()) {
              STPad *nextPad = rawEvent -> GetPad(row + 1, layer);
              pad = (!nextPad ? fPadArray -> at(0) : *nextPad);
            }

            goto whileStart;
          }
        }
      }
    }

    Double_t weightedRowSum = 0;
    Double_t chargeSum = 0;

    Int_t numLeft = row - fNumSidePads;
    numLeft = (numLeft < 0 ? 0 : numLeft);

    Int_t numRight = row + fNumSidePads;
    numRight = (numRight + 1 > fPadRows ? fPadRows - 1 : numRight);

    for (Int_t iRow = numLeft; iRow < numRight + 1; iRow++) {
      Int_t sideNumPeaks = fNumPeaks[iRow][layer];

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("iRow: %d layer: %d sideNumPeaks: %d!", iRow, layer, sideNumPeaks));
#endif

      if (!sideNumPeaks)
        continue;

      for (Int_t iPeak = 0; iPeak < sideNumPeaks; iPeak++) {
        Int_t sidePeakTb = fPeakTbs[iRow][layer][iPeak];
        Int_t sidePeakValue = fPeakValues[iRow][layer][iPeak];

        if (TMath::Abs(sidePeakTb - peakTb) < 3) {
          weightedRowSum += sidePeakValue*iRow;
          chargeSum += sidePeakValue;

          DeletePeakInfo(rawEvent, iRow, layer, iPeak);
        }
      }
    }

    Double_t xPos = CalculateX(weightedRowSum/chargeSum);

    Int_t selectedPoints = 0;
    Double_t selectedTbs[10] = {0};
    Double_t selectedValues[10] = {0};

#ifdef DEBUG
    fLogger -> Info(MESSAGE_ORIGIN, Form("Start finding the hit time of pad row: %d, layer: %d from peakTb: %d", pad.GetRow(), pad.GetLayer(), peakTb));
#endif

    for (Int_t iTb = peakTb; iTb > peakTb - 10; iTb--) {
      Double_t adc = pad.GetADC(iTb);

      if (adc < peakValue*fPercPeakMin/100. || adc > peakValue*fPercPeakMax/100.)
        continue;

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("peakTb: %d, peakValue: %f, selected iTb: %d, adc: %f", peakTb, peakValue, iTb, adc));
#endif

      selectedTbs[selectedPoints] = iTb;
      selectedValues[selectedPoints] = adc;
      selectedPoints++;
    }

    if (selectedPoints < fMinPoints) {

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("Too few points for fitting: %d!", selectedPoints));
#endif

      if (rawEvent -> GetNumPads())
        pad = fPadArray -> at(0);

      goto whileStart;
    }

    Double_t fitConst = 0;
    Double_t fitSlope = 0;

    LSLFit(selectedPoints, selectedTbs, selectedValues, fitConst, fitSlope);
    Double_t hitTime = -fitConst/fitSlope;

    Double_t yPos = CalculateY(hitTime);
    if (yPos > 0 || yPos < -fMaxDriftLength)
      continue;

    Double_t zPos = CalculateZ(layer);

    STHit *hit = new STHit(hitNum, xPos, yPos, zPos, peakValue);
    event -> AddHit(hit);
    delete hit;

    hitNum++;

    DeletePeakInfo(rawEvent, row, layer, 0);

#ifndef DEBUG
    manager.PrintOut(totalPads - fPadArray -> size());
#endif

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("Found hit %d, (x, y, z) = (%.2f, %.2f, %.2f), charge = %.2f!", hitNum, xPos, yPos, zPos, peakValue));
#endif
  }

#ifndef DEBUG
  manager.End();
#endif

}

void
STPSALayer::Reset()
{
  fPadArray = NULL;

  for (Int_t iRow = 0; iRow < fPadRows; iRow++) {
    for (Int_t iLayer = 0; iLayer < fPadLayers; iLayer++) {
      fNumPeaks[iRow][iLayer] = 0;

      for (Int_t iPeak = 0; iPeak < fPeakStorageSize; iPeak++) {
        fPeakTbs[iRow][iLayer][iPeak] = 0;
        fPeakValues[iRow][iLayer][iPeak] = 0;
      }
    }
  }
}

void
STPSALayer::PreAnalyze(STRawEvent *rawEvent)
{
#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, "Start pre-analyzing!");
#endif

  Reset();

  fPadArray = rawEvent -> GetPads();
  
  Int_t numPads = rawEvent -> GetNumPads();
  Int_t hitNum = 0;
  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad *pad = rawEvent -> GetPad(iPad);

    Int_t row = pad -> GetRow();
    Int_t layer = pad -> GetLayer();

    if (!(pad -> IsPedestalSubtracted())) {
      fLogger -> Error(MESSAGE_ORIGIN, "Pedestal should be subtracted to use this class!");

      std::exit(0);
    }

    Double_t *adc = pad -> GetADC();

    Float_t floatADC[512] = {0};
    Float_t dummy[512] = {0};
    for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
      floatADC[iTb] = adc[iTb];

    Int_t numPeaks = fPeakFinder -> SearchHighRes(floatADC, dummy, fNumTbs, 4.7, 5, kFALSE, 3, kTRUE, 3);

    if (numPeaks == 0)
      continue;

    fNumPeaks[row][layer] = numPeaks;

    for (Int_t iPeak = 0; iPeak < numPeaks; iPeak++) {
      Int_t peakTb = (Int_t)ceil((fPeakFinder -> GetPositionX())[iPeak]);
      Double_t peakAdc = adc[peakTb];

      fPeakTbs[row][layer][iPeak] = peakTb;
      fPeakValues[row][layer][iPeak] = peakAdc;

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("row: %d, layer: %d, tb: %d, adc: %f", pad -> GetRow(), pad -> GetLayer(), peakTb, peakAdc));
#endif

    }
  }

#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, "Pre-analyze completed!!");
#endif
}

void
STPSALayer::DeletePeakInfo(STRawEvent *rawEvent, Int_t row, Int_t layer, Int_t peakNum)
{
  if (!fNumPeaks[row][layer]) {

#ifdef DEBUG
    fLogger -> Info(MESSAGE_ORIGIN, Form("Delete pad row: %d, layer: %d!", row, layer));
#endif

    rawEvent -> RemovePad(row , layer);

    return;
  }

#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, Form("Delete peak info - row: %d, layer: %d, peakNum: %d!", row, layer, peakNum));
#endif

  for (Int_t iPeak = peakNum; iPeak < fNumPeaks[row][layer]; iPeak++) {
    fPeakTbs[row][layer][iPeak] = fPeakTbs[row][layer][iPeak + 1];
    fPeakValues[row][layer][iPeak] = fPeakValues[row][layer][iPeak + 1];
  }

  fNumPeaks[row][layer]--;
}

void
STPSALayer::LSLFit(Int_t numPoints, Double_t *x, Double_t *y, Double_t &constant, Double_t &slope)
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
}
