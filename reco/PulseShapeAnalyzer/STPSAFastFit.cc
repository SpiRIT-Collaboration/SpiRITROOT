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

ClassImp(STPSAFastFit)

STPSAFastFit::STPSAFastFit()
{
  fPulseData = new Double_t[2000];

  fNumTbsCompare = 12;

  std::ifstream ifpulse("../../parameters/PulserFitted.dat");
  Int_t countData = 0;
  while (ifpulse >> fPulseData[countData++]) {}

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
  hitArray -> Clear("C");
  Int_t hitNum = 0;

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

    Double_t xPos = (pad -> GetRow() + 0.5) * fPadSizeX - fPadPlaneX/2.;
    Double_t zPos = (pad -> GetLayer() + 0.5) * fPadSizeZ;
    //Double_t charge = 0;

    Double_t *adc = pad -> GetADC();

    Int_t numPeaks = 0;
    Int_t countRise = 0;

    for (Int_t iTb = 1; iTb < fNumTbsWindow; iTb++)
    {
      Double_t diff = adc[iTb] - adc[iTb - 1];

      if (adc[iTb] > fThreshold && diff > 2) {
        countRise++;
      }
      else {
        if (countRise > 4) { // fit and remove
          Int_t tbPeak = iTb - 1;
          Int_t tbStartTemp = tbPeak - countRise;

          while (adc[tbStartTemp] < adc[tbPeak] * 0.1)
            tbStartTemp++;

          Double_t tbStart = 0;
          Double_t chi2 = 0;
          Double_t amplitude = 0;

          Double_t chi2Temp = 0;
          Double_t amplitudeTemp= 0;

          Int_t iterate = 0;

          if (adc[tbPeak] > 3500) {
            amplitude = adc[iTb-1];
            tbStart = tbPeak - countRise;
          }
          else
          {
            tbStart = tbStartTemp;
            Bool_t increaseTbFlag = kFALSE;

            FitPulse(adc, tbStart, chi2, amplitude);
            FitPulse(adc, tbStart + 0.1, chi2Temp, amplitudeTemp);

            if (chi2Temp < chi2) {
              chi2 = chi2Temp;
              tbStart = tbStart + 0.1;
              amplitude = amplitudeTemp;
              increaseTbFlag = kTRUE;
            }

            iterate = 0;

            if (increaseTbFlag == kTRUE) 
            {
              while (1) 
              {
                if (iterate > 20)
                  break;

                FitPulse(adc, tbStart + 0.1, chi2Temp, amplitudeTemp);
                if (chi2Temp < chi2) {
                  chi2 = chi2Temp;
                  tbStart += 0.1;
                  amplitude = amplitudeTemp;
                }
                else 
                  break;

                iterate++;
              }
            }

            else 
            {
              while (1)
              {
                if (iterate > 20)
                  break;

                FitPulse(adc, tbStart - 0.1, chi2Temp, amplitudeTemp);
                if (chi2Temp < chi2) {
                  chi2 = chi2Temp;
                  tbStart -= 0.1;
                  amplitude = amplitudeTemp;
                }
                else 
                  break;

                iterate++;
              }
            }
          }

          if (amplitude < fThreshold)
            continue;

          Double_t yHit = -tbStart * fTBTime * fDriftVelocity / 100.;

          STHit *hit = (STHit *) hitArray -> ConstructedAt(hitNum);
          hit -> Clear();
          hit -> SetHit(hitNum, xPos, yHit, zPos, amplitude);
          hit -> SetRow(pad -> GetRow());
          hit -> SetLayer(pad -> GetLayer());
          hit -> SetTb(tbStart);
          hit -> SetChi2(chi2);
          hit -> SetNDF(fNumTbsCompare);

          hitNum++;

          for (Int_t iTbPulse = 0; iTbPulse < 100; iTbPulse++) {
            Int_t tb = tbStartTemp + iTbPulse;
            adc[tb] -= Pulse(tb + 0.5, amplitude, tbStart);
          }

          iTb = Int_t(tbStart) + 9;
        }

        countRise = 0;
      }
    }
  }

#ifdef DEBUG
  LOG(INFO) << "Thread ended!" << FairLogger::endl;
#endif
}

Double_t 
STPSAFastFit::Pulse(Double_t x, Double_t amp, Double_t tb)
{
  if (x < tb) 
    return 0;

  Double_t hit_time = x - tb;
  Int_t hit_time_0p1 = hit_time * 10;
  if (hit_time_0p1 > 1998) 
    return 0;

  Double_t r = 10 * hit_time - hit_time_0p1;
  Double_t val = r * fPulseData[hit_time_0p1 + 1] + (1 - r) * fPulseData[hit_time_0p1];

  return amp * val;
}

void 
STPSAFastFit::FitPulse(Double_t *buffer, Double_t tbStart, Double_t &chi2, Double_t &amp)
{
  Double_t refy = 0;
  Double_t ref2 = 0;

  for (Int_t iTbPulse = 0; iTbPulse < fNumTbsCompare; iTbPulse++) {
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

  amp = refy / ref2;

  chi2 = 0;
  for (Int_t iTbPulse = 0; iTbPulse < fNumTbsCompare; iTbPulse++) {
    Int_t tb = tbStart + iTbPulse;
    Double_t val = buffer[tb];
    Double_t ref = Pulse(tb + 0.5, amp, tbStart);
    chi2 += (val - ref) * (val - ref);
  }
}
