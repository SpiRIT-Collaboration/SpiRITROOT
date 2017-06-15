// SpiRITROOT classes
#include "STPSAGrad.hh"

// STL
#include <cmath>
#include <thread>
#include <iostream>

using namespace std;

// ROOT
#include "RVersion.h"

ClassImp(STPSAGrad)

STPSAGrad::STPSAGrad()
{
  Init();
}

STPSAGrad::~STPSAGrad()
{
}

void
STPSAGrad::Init()
{
  fThreadHitArray = new TClonesArray*[NUMTHREAD];
  for (Int_t iThread = 0; iThread < NUMTHREAD; iThread++)
    fThreadHitArray[iThread] = new TClonesArray("STHit", 100);

  fPadIndex = 0;
  fNumPads = 0;
  
  if (fWindowStartTb == 0)
    fWindowStartTb = 1;

  fTbStartCut = 270;
  fNumAscending = 4;

  fThresholdOneTbStep = 0.127421*fThreshold;
}

void STPSAGrad::Analyze(STRawEvent *rawEvent, STEvent *event)
{
}

void
STPSAGrad::Analyze(STRawEvent *rawEvent, TClonesArray *hitArray)
{
  fNumPads = rawEvent -> GetNumPads();
  fPadArray = rawEvent -> GetPads();
  fPadIndex = 0;

  std::thread thread[NUMTHREAD];
  for (Int_t iThread = 0; iThread < NUMTHREAD; iThread++)
    thread[iThread] = std::thread([this](TClonesArray *array) { this -> PadAnalyzer(array); }, fThreadHitArray[iThread]);

  for (Int_t iThread = 0; iThread < NUMTHREAD; iThread++) {

      thread[iThread].join();
  }

  Int_t hitNum = 0;
  for (Int_t iThread = 0; iThread < NUMTHREAD; iThread++) {
    Int_t numHits = fThreadHitArray[iThread] -> GetEntriesFast();

    for (Int_t iHit = 0; iHit < numHits; iHit++) {
      STHit *hit = (STHit *) fThreadHitArray[iThread] -> At(iHit);
      hit -> SetHitID(hitNum++);

      Double_t x = hit -> GetZ();
      Double_t y = hit -> GetY();

      new ((*hitArray)[hitArray->GetEntriesFast()]) STHit(hit);
    }
  }
}

void STPSAGrad::PadAnalyzer(TClonesArray *hitArray)
{
  Int_t hitNum = 0;

  hitArray -> Clear("C");

  while (1) {
    STPad *pad = nullptr;

    {
      std::lock_guard<std::mutex> lock(fMutex);
      if (fPadIndex == fNumPads) {

        return;
      }

      pad = &(fPadArray -> at(fPadIndex++));

      if (pad -> GetLayer() <= fLayerLowCut || pad -> GetLayer() >= fLayerHighCut )
        continue;
    }

    FindHits(pad, hitArray, hitNum);
  }
}

void 
STPSAGrad::FindHits(STPad *pad, TClonesArray *hitArray, Int_t &hitNum)
{
  Double_t *adcSource = pad -> GetADC();
  Double_t adc[512] = {0};
  memcpy(&adc, adcSource, sizeof(Double_t)*fNumTbs);

  // Pad information
  Int_t row   = pad -> GetRow();
  Int_t layer = pad -> GetLayer();
  Double_t xPos = (row   + 0.5) * fPadSizeX - fPadPlaneX/2.;
  Double_t zPos = (layer + 0.5) * fPadSizeZ;

  // Found peak information
  Int_t tbCurrent = fWindowStartTb;
  Int_t tbStart;

  // Fitted hit information
  Double_t yHit;
  Double_t grad;

  // Previous hit information

  while (FindPeak(adc, tbCurrent, tbStart)) 
  {
    if (tbStart > fTbStartCut - 1)
      break;

    FindMaxGrad(adc, tbCurrent, grad);

    yHit = tbCurrent * fTbToYConv;

    STHit *hit = (STHit *) hitArray -> ConstructedAt(hitNum);
    hit -> Clear();
    hit -> SetHit(hitNum, xPos, yHit, zPos, grad);
    hit -> SetRow(row);
    hit -> SetLayer(layer);
    hit -> SetTb(tbCurrent);
    hit -> SetChi2(0);
    hit -> SetNDF(0);
    hitNum++;

    tbCurrent = tbCurrent + 9;
  }
}

Bool_t
STPSAGrad::FindPeak(Double_t *adc, Int_t &tbCurrent, Int_t &tbStart) {
  Int_t countAscending      = 0;
  Int_t countAscendingBelow = 0;

  for (; tbCurrent < fWindowEndTb; tbCurrent++)
  {
    Double_t diff = adc[tbCurrent] - adc[tbCurrent - 1];

    // If adc difference of step is above threshold
    if (diff > fThresholdOneTbStep) 
    {
      if (adc[tbCurrent] > fThreshold) countAscending++;
      else countAscendingBelow++;
    }
    else 
    {
      // If acended step is below 5, 
      // or negative pulse is bigger than the found pulse, continue
      if (countAscending < fNumAscending || ((countAscendingBelow >= countAscending) && (-adc[tbCurrent - 1 - countAscending - countAscendingBelow] > adc[tbCurrent - 1]))) 
      {
        countAscending = 0;
        countAscendingBelow = 0;
        continue;
      }

      tbCurrent -= 1;
      if (adc[tbCurrent] < fThreshold)
        continue;

      // Peak is found!
      tbStart = tbCurrent - countAscending;
      while (adc[tbStart] < adc[tbCurrent] * 0.05)
        tbStart++;

      return kTRUE;
    }
  }

  return kFALSE;
}

void
STPSAGrad::FindMaxGrad(Double_t *adc, Int_t tbCurrent, Double_t &grad)
{
  grad = -9999;

  while (1) {
    if (tbCurrent <= 1 || adc[tbCurrent] < fThreshold)
      break;

    auto thisGrad = adc[tbCurrent] - adc[tbCurrent - 1];
    if (thisGrad > grad)
      grad = thisGrad;

    tbCurrent--;
  }
}
