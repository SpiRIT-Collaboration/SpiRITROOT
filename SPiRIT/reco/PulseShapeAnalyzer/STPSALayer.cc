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

// STL
#include <cmath>
#include <vector>
#include <cstdlib>

// ROOT classes
#include "TH2D.h"

ClassImp(STPSALayer)

STPSALayer::STPSALayer()
{
  fPeakFinder = new TSpectrum();

  fNumSidePads = 2;
  fPeakStorageSize = 50;

  fMinPoints = 4;
  fPercPeakMin = 10;
  fPercPeakMax = 90;

  fPadID = new Short_t*[fPadRows];
  fNumPeaks = new Short_t*[fPadRows];
  fPeakTbs = new Short_t**[fPadRows];
  fPeakValues = new Short_t**[fPadRows];
  for (Int_t iRow = 0; iRow < fPadRows; iRow++) {
    fPadID[iRow] = new Short_t[fPadLayers];
    fNumPeaks[iRow] = new Short_t[fPadLayers];
    fPeakTbs[iRow] = new Short_t *[fPadLayers];
    fPeakValues[iRow] = new Short_t *[fPadLayers];
    for (Int_t iLayer = 0; iLayer < fPadLayers; iLayer++) {
      fPadID[iRow][iLayer] = 0;
      fNumPeaks[iRow][iLayer] = 0;
      fPeakTbs[iRow][iLayer] = new Short_t[fPeakStorageSize];
      fPeakValues[iRow][iLayer] = new Short_t[fPeakStorageSize];
      for (Int_t iPeak = 0; iPeak < fPeakStorageSize; iPeak++) {
        fPeakTbs[iRow][iLayer][iPeak] = 0;
        fPeakValues[iRow][iLayer][iPeak] = 0;
      }
    }
  }
}

STPSALayer::~STPSALayer()
{
}

void
STPSALayer::Analyze(STRawEvent *rawEvent, STEvent *event)
{
  PreAnalyze(rawEvent);

  Int_t hitNum = 0;

  STPad *pad = NULL;
  Int_t padID = 0;
  whileStart:
  while (rawEvent -> GetNumPads()) {
    pad = rawEvent -> GetPad(padID);

    Int_t row = pad -> GetRow();
    Int_t layer = pad -> GetLayer();

    Short_t &numPeaks = fNumPeaks[row][layer];
    if (!numPeaks) {
      rawEvent -> RemovePad(padID);

      goto whileStart;
    }

    Short_t peakTb = fPeakTbs[row][layer][0];
    Short_t peakValue = fPeakValues[row][layer][0];

    if (peakValue < fThreshold) {
      DeletePeakInfo(rawEvent, row, layer, 0);

      goto whileStart;
    }

    if (!(row == 107)) {
      Int_t leftNumPeaks = fNumPeaks[row - 1][layer];
      if (leftNumPeaks) {
        for (Int_t iPeak = 0; iPeak < leftNumPeaks; iPeak++) {
          Short_t leftPeakTb = fPeakTbs[row - 1][layer][iPeak];
          Short_t leftPeakValue = fPeakValues[row - 1][layer][iPeak];

          if (TMath::Abs(leftPeakTb - peakTb) < 3 && peakValue < leftPeakValue) {
            padID = fPadID[row - 1][layer];

            goto whileStart;
          }
        }
      }
    }

    if (!(row == 0)) {
      Int_t rightNumPeaks = fNumPeaks[row + 1][layer];
      if (rightNumPeaks) {
        for (Int_t iPeak = 0; iPeak < rightNumPeaks; iPeak++) {
          Short_t rightPeakTb = fPeakTbs[row + 1][layer][iPeak];
          Short_t rightPeakValue = fPeakValues[row + 1][layer][iPeak];

          if (TMath::Abs(rightPeakTb - peakTb) < 3 && peakValue < rightPeakValue) {
            padID = fPadID[row + 1][layer];

            goto whileStart;
          }
        }
      }
    }

    Double_t weightedRowSum = 0;
    Double_t chargeSum = 0;

    weightedRowSum += peakValue*row;
    chargeSum += peakValue;

    Int_t numLeft = row - fNumSidePads;
    numLeft = (numLeft < 0 ? 0 : numLeft);

    Int_t numRight = row + fNumSidePads;
    numRight = (numRight > 107 ? 107 : numRight);

    for (Int_t iRow = numLeft; iRow < numRight + 1; iRow++) {
      Int_t sideNumPeaks = fNumPeaks[iRow][layer];

      if (!sideNumPeaks)
        continue;

      for (Int_t iPeak = 0; iPeak < sideNumPeaks; iPeak++) {
        Short_t sidePeakTb = fPeakTbs[iRow][layer][sideNumPeaks - iPeak - 1];
        Short_t sidePeakValue = fPeakValues[iRow][layer][sideNumPeaks - iPeak - 1];

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

    for (Int_t iTb = peakTb; iTb > peakTb - 10; iTb--) {
      Double_t adc = pad -> GetADC(iTb);

      if (adc < peakValue*fPercPeakMin/100. || adc > peakValue*fPercPeakMax/100.)
        continue;

      selectedTbs[selectedPoints] = iTb;
      selectedValues[selectedPoints] = adc;
      selectedPoints++;
    }

    if (selectedPoints < fMinPoints) {
      DeletePeakInfo(rawEvent, row, layer, peakTb);

      goto whileStart;
    }

    Double_t fitConst= 0;
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
  }
}

void
STPSALayer::Reset()
{
  for (Int_t iRow = 0; iRow < fPadRows; iRow++) {
    for (Int_t iLayer = 0; iLayer < fPadLayers; iLayer++) {
      fPadID[iRow][iLayer] = 0;
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
  Reset();
  
  Int_t numPads = rawEvent -> GetNumPads();
  Int_t hitNum = 0;
  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad *pad = rawEvent -> GetPad(iPad);

    Short_t row = pad -> GetRow();
    Short_t layer = pad -> GetLayer();

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

    fPadID[row][layer] = iPad;

    fNumPeaks[row][layer] = numPeaks;

    for (Int_t iPeak = 0; iPeak < numPeaks; iPeak++) {
      Short_t peakTb = (Int_t)ceil((fPeakFinder -> GetPositionX())[numPeaks - iPeak - 1]);
      Double_t peakAdc = adc[peakTb];

      fPeakTbs[row][layer][iPeak] = peakTb;
      fPeakValues[row][layer][iPeak] = peakAdc;
    }
  }
}

void
STPSALayer::DeletePeakInfo(STRawEvent *rawEvent, Short_t row, Short_t layer, Short_t peakNum)
{
  for (Int_t iPeak = peakNum; iPeak < fNumPeaks[row][layer]; iPeak++) {
    fPeakTbs[row][layer][iPeak] = fPeakTbs[row][layer][iPeak + 1];
    fPeakValues[row][layer][iPeak] = fPeakValues[row][layer][iPeak + 1];
  }

  fNumPeaks[row][layer]--;

  if (!fNumPeaks[row][layer])
    rawEvent -> RemovePad(fPadID[row][layer]);
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
