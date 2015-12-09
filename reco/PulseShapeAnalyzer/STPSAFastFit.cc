// SpiRITROOT classes
#include "STPSAFastFit.hh"
#include "STGlobal.hh"

// STL
#include <cmath>
#include <thread>
#include <iostream>

using namespace std;

// ROOT
#include "RVersion.h"

//#define DEBUG
//#define DEBUG_PEAKFINDING
//#define DEBUG_FIT
//#define DEBUG_NAN

ClassImp(STPSAFastFit)

STPSAFastFit::STPSAFastFit()
{
  fNDFTbs = 12;
  fIterMax = 40;
  fNumTbsCorrection = 50;

  fThreadHitArray = new TClonesArray*[NUMTHREAD];
  for (Int_t iThread = 0; iThread < NUMTHREAD; iThread++)
    fThreadHitArray[iThread] = new TClonesArray("STHit", 100);

  fPadReady = kFALSE;
  fPadTaken = kFALSE;
  fEnd = kFALSE;
}

void
STPSAFastFit::Analyze(STRawEvent *rawEvent, STEvent *event)
{
  fPadReady = kFALSE;
  fPadTaken = kFALSE;
  fEnd = kFALSE;

  Int_t numPads = rawEvent -> GetNumPads();

#ifdef DEBUG
  LOG(INFO) << "Start to create threads!" << FairLogger::endl;
#endif

  std::thread thread[NUMTHREAD];
  for (Int_t iThread = 0; iThread < NUMTHREAD; iThread++)
    thread[iThread] = std::thread([this](TClonesArray *array) { this -> PadAnalyzer(array); }, fThreadHitArray[iThread]);

#ifdef DEBUG
  LOG(INFO) << "Successfully created threads!" << FairLogger::endl;
#endif

  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    std::unique_lock<std::mutex> lock(fMutex);

    fPad = rawEvent -> GetPad(iPad);

    if (fPad -> GetLayer() <= fLayerCut) {
      lock.unlock();
      continue;
    }

    fPadReady = kTRUE;
    fPadTaken = kFALSE;

#ifdef DEBUG
  LOG(INFO) << "Passing pad: " << iPad << FairLogger::endl;
#endif

    fCondVariable.notify_one();
    fCondVariable.wait(lock, [this] { return fPadTaken; });
  }

#ifdef DEBUG
  LOG(INFO) << "Scheduling analysis is done! Join the threads." << FairLogger::endl;
#endif

  fEnd = kTRUE;
  for (Int_t iThread = 0; iThread < NUMTHREAD; iThread++) {

#ifdef DEBUG
  LOG(INFO) << "Thread: " << iThread << " is not joinable!"  << FairLogger::endl;
#endif

      fCondVariable.notify_all();
      thread[iThread].join();
  }

#ifdef DEBUG
  LOG(INFO) << "Joining completed! Merging data!"  << FairLogger::endl;
#endif

  Int_t hitNum = 0;
  for (Int_t iThread = 0; iThread < NUMTHREAD; iThread++) {
    Int_t numHits = fThreadHitArray[iThread] -> GetEntriesFast();

    for (Int_t iHit = 0; iHit < numHits; iHit++) {
      STHit *hit = (STHit *) fThreadHitArray[iThread] -> At(iHit);
      hit -> SetHitID(hitNum++);
      event -> AddHit(hit);
    }
  }
}

void STPSAFastFit::PadAnalyzer(TClonesArray *hitArray)
{
  Int_t hitNum = 0;

  hitArray -> Clear("C");

  while (1) {
    std::unique_lock<std::mutex> lock(fMutex);
    fCondVariable.wait(lock, [this] { return (fEnd ? fEnd : fPadReady); });

    if (fEnd) {

#ifdef DEBUG
    LOG(INFO) << "End signal received!" << FairLogger::endl;
#endif

      break;
    }

    STPad *pad = fPad; 
    fPad = NULL;

    fPadReady = kFALSE;
    fPadTaken = kTRUE;

    lock.unlock();
    fCondVariable.notify_all();

    FindHits(pad, hitArray, hitNum);
  }

#ifdef DEBUG
  LOG(INFO) << "Thread ended!" << FairLogger::endl;
#endif
}

void 
STPSAFastFit::FindHits(STPad *pad, TClonesArray *hitArray, Int_t &hitNum)
{
  Double_t *adcSource = pad -> GetADC();
  Double_t adc[512] = {0};
  memcpy(&adc, adcSource, sizeof(Double_t)*fNumTbs);

  Int_t row = pad -> GetRow();
  Int_t layer = pad -> GetLayer();
  Double_t xPos = (row + 0.5) * fPadSizeX - fPadPlaneX/2.;
  Double_t zPos = (layer + 0.5) * fPadSizeZ;

  Int_t countAscending = 0;
  Int_t countAscendingBelow = 0;

  Double_t amplitudePrePeak = 0;
  Double_t tbStartPrePeak = 0;

  for (Int_t iTb = fWindowStartTb; iTb < fWindowEndTb; iTb++)
  {
    Double_t diff = adc[iTb] - adc[iTb - 1];

    if (diff > 2) {
      if (adc[iTb] > fThreshold)
        countAscending++;
      else
        countAscendingBelow++;
    }
    else {
      if (countAscending < 5 || ((countAscendingBelow >= countAscending) && (-adc[iTb - 1 - countAscending - countAscendingBelow] > adc[iTb - 1]))) {
        countAscending = 0;
        countAscendingBelow = 0;
        continue;
      }

      Int_t tbPeak = iTb - 1;
      Int_t tbStart = tbPeak - countAscending;

#ifdef DEBUG_PEAKFINDING
      LOG(INFO) << "Found peak " << tbPeak 
                << ", starting from " << tbStart
                << ", ascended " << countAscending
                << ", ascended-below " << countAscendingBelow
                << ", peak " << adc[iTb - 1]
                << ", below-peak " << adc[iTb - 1 - countAscendingBelow] << FairLogger::endl;
#endif

      while (adc[tbStart] < adc[tbPeak] * 0.05){
        tbStart++;
      }

      Double_t tbStartPulse = tbStart + 0.5;

      Double_t amplitude = 0;
      Double_t chi2 = 0;
      Int_t ndf = fNDFTbs;

      Double_t chi2Temp = 0;
      Double_t amplitudeTemp = 0;

      Int_t countIteration = 0;

      if (adc[tbPeak] > 3500) 
      {
        ndf = tbPeak - tbStart + 1;
        FitPulse(adc, tbStartPulse, ndf, chi2, amplitude);
#ifdef DEBUG_PEAKFINDING
        LOG(INFO) << "Found saturated hit! (" << hitNum << ")"
                  << " amp:" << amplitude 
                  << " tb:" << tbStartPulse << "~" << tbStartPulse + ndf 
                  << " chi2:" << chi2 << FairLogger::endl;
#endif
      }
      else
      {
        Bool_t increasingTbFlag = kFALSE;

        FitPulse(adc, tbStartPulse,       ndf, chi2,     amplitude);
        FitPulse(adc, tbStartPulse + 0.1, ndf, chi2Temp, amplitudeTemp);

        if (chi2Temp < chi2) {
          chi2 = chi2Temp;
          tbStartPulse = tbStartPulse + 0.1;
          amplitude = amplitudeTemp;
          increasingTbFlag = kTRUE;
        }

        countIteration = 2;

        if (increasingTbFlag == kTRUE) 
        {
          while (1) 
          {
            if (countIteration > fIterMax)
              break;

            countIteration++;

            FitPulse(adc, tbStartPulse + 0.1, ndf, chi2Temp, amplitudeTemp);
            if (chi2Temp < chi2) {
              chi2 = chi2Temp;
              tbStartPulse += 0.1;
              amplitude = amplitudeTemp;
            }
            else 
              break;
          }
        }

        else 
        {
          while (1)
          {
            if (countIteration > fIterMax)
              break;

            countIteration++;

            FitPulse(adc, tbStartPulse - 0.1, ndf, chi2Temp, amplitudeTemp);
            if (chi2Temp < chi2) {
              chi2 = chi2Temp;
              tbStartPulse -= 0.1;
              amplitude = amplitudeTemp;
            }
            else 
              break;
          }
        }

#ifdef DEBUG_PEAKFINDING
        LOG(INFO) << "Test hit! (" << hitNum << ")"
                  << " amp:" << amplitude 
                  << " tb:" << tbStartPulse << "~" << tbStartPulse + ndf 
                  << " chi2:" << chi2 
                  << " iter:" << countIteration << FairLogger::endl;
#endif
        if (amplitude < fThreshold) {
          countAscending = 0;
          countAscendingBelow = 0;
          iTb = tbPeak;
#ifdef DEBUG_PEAKFINDING
          LOG(INFO) << "Amplitude smaller than threshold, "
                    << amplitude << " < " << fThreshold
                    << FairLogger::endl;
          LOG(INFO) << "Restart peak finding from " << iTb << FairLogger::endl;
#endif
          continue;
        }

        if (amplitude < Pulse(tbStartPulse + 9, amplitudePrePeak, tbStartPrePeak) / 2.5) {
          countAscending = 0;
          countAscendingBelow = 0;
          iTb = tbPeak;
#ifdef DEBUG_PEAKFINDING
          LOG(INFO) << "Previous peak shadows current peak, "
                    << amplitude << " < " 
                    << Pulse(tbStartPulse + 9, amplitudePrePeak, tbStartPrePeak) / 2.5 << FairLogger::endl;
          LOG(INFO) << "Restart peak finding from " << iTb << FairLogger::endl;
#endif

          for (Int_t iTbPulse = -1; iTbPulse < fNumTbsCorrection; iTbPulse++) {
            Int_t tb = Int_t(tbStartPulse) + iTbPulse;
            adc[tb] -= Pulse(tb, amplitude, tbStartPulse);
          }

          continue;
        }

        for (Int_t iTbPulse = -1; iTbPulse < fNumTbsCorrection; iTbPulse++) {
          Int_t tb = Int_t(tbStartPulse) + iTbPulse;
          adc[tb] -= Pulse(tb, amplitude, tbStartPulse);
        }
#ifdef DEBUG_PEAKFINDING
        LOG(INFO) << "Found hit! (" << hitNum << ")"
                  << " amp:" << amplitude 
                  << " tb:" << tbStartPulse << "~" << tbStartPulse + ndf 
                  << " chi2:" << chi2 
                  << " iter:" << countIteration << FairLogger::endl;
#endif
      }

      Double_t yHit = -tbStartPulse * fTBTime * fDriftVelocity / 100.;

#ifdef DEBUG_NAN
      if ((xPos == xPos) == kFALSE) LOG(INFO) << "xPos is NAN! " << FairLogger::endl;
      if ((yHit == yHit) == kFALSE) LOG(INFO) << "yHit is NAN! " << FairLogger::endl;
      if ((zPos == zPos) == kFALSE) LOG(INFO) << "zPos is NAN! " << FairLogger::endl;
      if ((amplitude == amplitude) == kFALSE) LOG(INFO) << "amplitude is NAN! " << FairLogger::endl;
      if ((tbStartPulse == tbStartPulse) == kFALSE) LOG(INFO) << "tbStartPulse is NAN! " << FairLogger::endl;
      if ((chi2 == chi2) == kFALSE) LOG(INFO) << "chi2 is NAN! " << FairLogger::endl;
#endif

      STHit *hit = (STHit *) hitArray -> ConstructedAt(hitNum);
      hit -> Clear();
      hit -> SetHit(hitNum, xPos, yHit, zPos, amplitude);
      hit -> SetRow(row);
      hit -> SetLayer(layer);
      hit -> SetTb(tbStartPulse);
      hit -> SetChi2(chi2);
      hit -> SetNDF(ndf);

      amplitudePrePeak = amplitude;
      tbStartPrePeak = tbStartPulse;

      hitNum++;
      countAscending = 0;
      countAscendingBelow = 0;
      iTb = Int_t(tbStartPulse) + 9;

#ifdef DEBUG_PEAKFINDING
      LOG(INFO) << "Restart peak finding from " << iTb << FairLogger::endl;
#endif
    }
  }
}

void 
STPSAFastFit::FitPulse(Double_t *buffer, Double_t tbStart, Int_t ndf, Double_t &chi2, Double_t &amplitude)
{
  Double_t refy = 0;
  Double_t ref2 = 0;

  for (Int_t iTbPulse = 0; iTbPulse < ndf; iTbPulse++) {
    Int_t tb = tbStart + iTbPulse;
    Double_t y = buffer[tb];

    Double_t ref = Pulse(tb + 0.5, 1, tbStart);
    refy += ref * y;
    ref2 += ref * ref;
  }

  if (ref2 == 0)
  {
    chi2 = 1.e10;
    return;
  }

  amplitude = refy / ref2;

  chi2 = 0;
  for (Int_t iTbPulse = 0; iTbPulse < ndf; iTbPulse++) {
    Int_t tb = tbStart + iTbPulse;
    Double_t val = buffer[tb];
    Double_t ref = Pulse(tb + 0.5, amplitude, tbStart);
    chi2 += (val - ref) * (val - ref);
#ifdef DEBUG_FIT
    LOG(INFO) << " tbStart:" << tbStart
              << " tb:" << tb 
              << " val:" << val 
              << " ref:" << ref 
              << " diff:" << val - ref << FairLogger::endl;
#endif
  }
#ifdef DEBUG_FIT
  LOG(INFO) << "==> tbStart:" << tbStart
            << " chi2:" << chi2 
            << " amp:" << amplitude << FairLogger::endl;
#endif
}
