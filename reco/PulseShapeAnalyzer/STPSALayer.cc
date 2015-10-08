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

// ROOT
#include "RVersion.h"

//#define DEBUG

ClassImp(STPSALayer)

STPSALayer::STPSALayer()
{

#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, "Start Initializing!");
#endif

  fPeakFinder = new TSpectrum();

  fNumFiredPads = 0;
  fArrayIdx = 0;

  fIgnoreLeft = kFALSE;
  fIgnoreRight = kFALSE;
  fPrevRightPeak.Reset();
  fPrevLeftPeak.Reset();

  fNumSidePads = 4;
  fNumSideTbs = 4;
  fPeakStorageSize = 50;

  fMinPoints = 4;
  fPercPeakMin = 10;
  fPercPeakMax = 90;

  fPadIdxArray = new Int_t[fPadLayers*fPadRows];
  fNumPeaks = new Int_t*[fPadRows];
  fPeaks = new STPeak**[fPadRows];
  for (Int_t iRow = 0; iRow < fPadRows; iRow++) {
    fNumPeaks[iRow] = new Int_t[fPadLayers];
    fPeaks[iRow] = new STPeak*[fPadLayers];
    for (Int_t iLayer = 0; iLayer < fPadLayers; iLayer++) {
      fPadIdxArray[iLayer*fPadRows + iRow] = -1;
      fNumPeaks[iRow][iLayer] = 0;
      fPeaks[iRow][iLayer] = new STPeak[fPeakStorageSize];
      for (Int_t iPeak = 0; iPeak < fPeakStorageSize; iPeak++)
        fPeaks[iRow][iLayer][iPeak].Reset();
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
  Reset();

  fPadArray = rawEvent -> GetPads();

  PreAnalyze();

#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, "Start analyzing the event!");
#endif

  Int_t hitNum = 0;

  Int_t padIdxCheck = GetUnusedPadIdx(); 
  if (padIdxCheck == -1) return;
  STPad pad = fPadArray -> at(padIdxCheck);
  Int_t totalPads = fNumFiredPads;

#ifndef DEBUG
  STProcessManager manager("STPSALayer", totalPads);
#endif

  whileStart:
  while (fNumFiredPads) {

#ifdef DEBUG
    fLogger -> Info(MESSAGE_ORIGIN, Form("Start with pad row: %d, layer: %d, numPeaks: %d, rest pads: %d", pad.GetRow(), pad.GetLayer(), fNumPeaks[pad.GetRow()][pad.GetLayer()], fNumFiredPads));
#endif

    Int_t row = pad.GetRow();
    Int_t layer = pad.GetLayer();

    Int_t numPeaks = fNumPeaks[row][layer];
    if (!numPeaks) {

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("No peak in pad row: %d, layer: %d!", pad.GetRow(), pad.GetLayer()));
#endif

      Int_t padIdx = GetUnusedPadIdx();
      if (padIdx == -1) break;
      else pad = fPadArray -> at(padIdx);

      goto whileStart;
    }

    Int_t peakTb = fPeaks[row][layer][0].tb;
    Double_t peakValue = fPeaks[row][layer][0].value;

    if (fIgnoreLeft) {
      Int_t iPeak = 0;
      while (1) { 
        peakTb = fPeaks[row][layer][iPeak].tb;
        peakValue = fPeaks[row][layer][iPeak].value;

        if (TMath::Abs(fPrevLeftPeak.tb - peakTb) < fNumSideTbs && peakValue >= fPrevLeftPeak.value)
          break;
        else {
          iPeak++;
          continue;
        }
      }
    } else if (fIgnoreRight) {
      Int_t iPeak = 0;
      while (1) { 
        peakTb = fPeaks[row][layer][iPeak].tb;
        peakValue = fPeaks[row][layer][iPeak].value;

        if (TMath::Abs(fPrevRightPeak.tb - peakTb) < fNumSideTbs && peakValue >= fPrevRightPeak.value)
          break;
        else {
          iPeak++;
          continue;
        }
      }
    }

    if (!(row == 0)) {

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("Checking left pad row: %d, layer: %d!", row - 1, layer));
#endif

      Int_t leftNumPeaks = fNumPeaks[row - 1][layer];
      if (leftNumPeaks) {
        for (Int_t iPeak = 0; iPeak < leftNumPeaks; iPeak++) {
          Int_t leftPeakTb = fPeaks[row - 1][layer][iPeak].tb;
          Double_t leftPeakValue = fPeaks[row - 1][layer][iPeak].value;

          if (TMath::Abs(leftPeakTb - peakTb) < fNumSideTbs && peakValue < leftPeakValue) {

#ifdef DEBUG
            fLogger -> Info(MESSAGE_ORIGIN, Form("peakTb: %d, peakValue: %f, left tb: %d, peakValue: %f!", peakTb, peakValue, leftPeakTb, leftPeakValue));
#endif

            pad = fPadArray -> at(fPadIdxArray[GetArrayIdx(row - 1, layer)]);
            fPrevRightPeak.index = fPeaks[row - 1][layer][iPeak].index;
            fPrevRightPeak.tb = leftPeakTb;
            fPrevRightPeak.value = leftPeakValue;
            fIgnoreRight = kTRUE;

            goto whileStart;
          }
        }
      }
    }
    
    fPrevRightPeak.Reset();
    fIgnoreLeft = kFALSE;

    if (!(row + 1 == fPadRows)) {

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("Checking right pad row: %d, layer: %d!", row + 1, layer));
#endif

      Int_t rightNumPeaks = fNumPeaks[row + 1][layer];
      if (rightNumPeaks) {
        for (Int_t iPeak = 0; iPeak < rightNumPeaks; iPeak++) {
          Int_t rightPeakTb = fPeaks[row + 1][layer][iPeak].tb;
          Double_t rightPeakValue = fPeaks[row + 1][layer][iPeak].value;

          if (TMath::Abs(rightPeakTb - peakTb) < fNumSideTbs && peakValue < rightPeakValue) {

#ifdef DEBUG
            fLogger -> Info(MESSAGE_ORIGIN, Form("peakTb: %d, peakValue: %f, right tb: %d, peakValue: %f!", peakTb, peakValue, rightPeakTb, rightPeakValue));
#endif

            pad = fPadArray -> at(fPadIdxArray[GetArrayIdx(row + 1, layer)]);
            fPrevLeftPeak.index = fPeaks[row + 1][layer][iPeak].index;
            fPrevLeftPeak.tb = rightPeakTb;
            fPrevLeftPeak.value = rightPeakValue;
            fIgnoreLeft = kTRUE;

            goto whileStart;
          }
        }
      }
    }

    fPrevLeftPeak.Reset();
    fIgnoreRight = kFALSE;

    if (peakValue < fThreshold) {

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("Peak isn't bigger than threshold: %f in pad row: %d, layer: %d!", peakValue, pad.GetRow(), pad.GetLayer()));
#endif

      DeletePeakInfo(row, layer, 0);

      Int_t padIdx = GetUnusedPadIdx();
      if (padIdx == -1) break;
      else pad = fPadArray -> at(padIdx);

      goto whileStart;
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
      fLogger -> Info(MESSAGE_ORIGIN, Form("iRow: %d, layer: %d, sideNumPeaks: %d!", iRow, layer, sideNumPeaks));
#endif

      if (!sideNumPeaks)
        continue;

      for (Int_t iPeak = 0; iPeak < sideNumPeaks; iPeak++) {
        Int_t sidePeakTb = fPeaks[iRow][layer][iPeak].tb;
        Int_t sidePeakValue = fPeaks[iRow][layer][iPeak].value;

        if (TMath::Abs(sidePeakTb - peakTb) < fNumSideTbs) {
          weightedRowSum += sidePeakValue*iRow;
          chargeSum += sidePeakValue;

          DeletePeakInfo(iRow, layer, iPeak);
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

      Int_t padIdx = GetUnusedPadIdx();
      if (padIdx == -1) break;
      else pad = fPadArray -> at(padIdx);

      goto whileStart;
    }

    Double_t fitConst = 0;
    Double_t fitSlope = 0;
    Double_t chi2 = 0;

    LSLFit(selectedPoints, selectedTbs, selectedValues, fitConst, fitSlope, chi2);
    Double_t hitTime = -fitConst/fitSlope;

    Double_t yPos = CalculateY(hitTime);
    if (yPos > 0 || yPos < -fMaxDriftLength)
      continue;

    Double_t zPos = CalculateZ(layer);

    STHit *hit = new STHit(hitNum, xPos, yPos, zPos, peakValue);
    hit -> SetRow(pad.GetRow());
    hit -> SetLayer(pad.GetLayer());
    hit -> SetTb(hitTime);
    hit -> SetChi2(chi2);
    hit -> SetNDF(selectedPoints);
    event -> AddHit(hit);
    delete hit;

    hitNum++;

#ifndef DEBUG
    manager.PrintOut(totalPads - fNumFiredPads);
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
  fNumFiredPads = 0;
  fArrayIdx = 0;

  fIgnoreLeft = kFALSE;
  fIgnoreRight = kFALSE;

  fPrevRightPeak.Reset();
  fPrevLeftPeak.Reset();

  fPadArray = NULL;

  for (Int_t iRow = 0; iRow < fPadRows; iRow++) {
    for (Int_t iLayer = 0; iLayer < fPadLayers; iLayer++) {
      fPadIdxArray[iLayer*fPadRows + iRow] = -1;
      fNumPeaks[iRow][iLayer] = 0;

      for (Int_t iPeak = 0; iPeak < fPeakStorageSize; iPeak++)
        fPeaks[iRow][iLayer][iPeak].Reset();
    }
  }
}

Int_t
STPSALayer::GetArrayIdx(Int_t row, Int_t layer)
{
  return (layer*fPadRows + row);
}

Int_t
STPSALayer::GetUnusedPadIdx()
{
  Int_t row = fArrayIdx%fPadRows;
  Int_t layer = fArrayIdx/fPadRows;

  if (fNumPeaks[row][layer]) {

#ifdef DEBUG
    fLogger -> Info(MESSAGE_ORIGIN, Form("Unused fArrayIdx: %d, pad row: %d, layer: %d, fPadIdxArray: %d!", fArrayIdx, row, layer, fPadIdxArray[fArrayIdx]));
#endif

    return fPadIdxArray[fArrayIdx];
  } else {
    fArrayIdx++;

    if (fArrayIdx > 12096)
      return -1;
    
    return GetUnusedPadIdx();
  }
}

void
STPSALayer::PreAnalyze()
{
#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, "Start pre-analyzing!");
#endif
  
  Int_t numPads = fPadArray -> size();
  Int_t hitNum = 0;
  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad pad = fPadArray -> at(iPad);

    Int_t row = pad.GetRow();
    Int_t layer = pad.GetLayer();

    if (!(pad.IsPedestalSubtracted())) {
      fLogger -> Error(MESSAGE_ORIGIN, "Pedestal should be subtracted to use this class!");

      std::exit(0);
    }

    Double_t *adc = pad.GetADC();

#if ROOT_VERSION_CODE < ROOT_VERSION(6,0,0)
    Float_t floatADC[512] = {0};
    Float_t dummy[512] = {0};
    for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
      floatADC[iTb] = adc[iTb];

    Int_t numPeaks = fPeakFinder -> SearchHighRes(floatADC, dummy, fNumTbs, 4.7, 5, kFALSE, 3, kTRUE, 3);
#else
    Double_t dummy[512] = {0};

    Int_t numPeaks = fPeakFinder -> SearchHighRes(adc, dummy, fNumTbs, 4.7, 5, kFALSE, 3, kTRUE, 3);
#endif

    if (numPeaks == 0)
      continue;

    fNumFiredPads++;
    fPadIdxArray[layer*fPadRows + row] = iPad;
    fNumPeaks[row][layer] = numPeaks;

    for (Int_t iPeak = 0; iPeak < numPeaks; iPeak++) {
      Int_t peakTb = (Int_t)ceil((fPeakFinder -> GetPositionX())[iPeak]);
      Double_t peakAdc = adc[peakTb];

      fPeaks[row][layer][iPeak].index = iPad;
      fPeaks[row][layer][iPeak].tb = peakTb;
      fPeaks[row][layer][iPeak].value = peakAdc;

#ifdef DEBUG
      fLogger -> Info(MESSAGE_ORIGIN, Form("row: %d, layer: %d, tb: %d, adc: %f", pad.GetRow(), pad.GetLayer(), peakTb, peakAdc));
#endif

    }
  }

#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, "Pre-analyze completed!!");
#endif
}

void
STPSALayer::DeletePeakInfo(Int_t row, Int_t layer, Int_t peakNum)
{
  if (fNumPeaks[row][layer]) {

#ifdef DEBUG
    fLogger -> Info(MESSAGE_ORIGIN, Form("Delete peak info - row: %d, layer: %d, peakNum: %d!", row, layer, peakNum));
#endif

    for (Int_t iPeak = peakNum; iPeak < fNumPeaks[row][layer]; iPeak++) {
      fPeaks[row][layer][iPeak].tb = fPeaks[row][layer][iPeak + 1].tb;
      fPeaks[row][layer][iPeak].value = fPeaks[row][layer][iPeak + 1].value;
    }

    fNumPeaks[row][layer]--;
  }

  if (!fNumPeaks[row][layer]) {

#ifdef DEBUG
    fLogger -> Info(MESSAGE_ORIGIN, Form("Delete pad row: %d, layer: %d!", row, layer));
#endif

    fNumFiredPads--;

    return;
  }
}
