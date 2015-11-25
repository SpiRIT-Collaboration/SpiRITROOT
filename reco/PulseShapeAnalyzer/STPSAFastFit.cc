// SpiRITROOT classes
#include "STPSAFastFit.hh"
#include "STGlobal.hh"

// STL
#include <cmath>
#include <thread>

// ROOT
#include "RVersion.h"

//#define DEBUG

ClassImp(STPSAFastFit)

STPSAFastFit::STPSAFastFit()
{
  fPulseData = new Double_t[2000];
  fPulse = new TF1("pulse", this, &STPSAFastFit::Pulse, 0, fNumTbsWindow, 2, "STPSAFastFit", "Pulse");

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
  TSpectrum peakFinder;

  auto calculateX = [this](Double_t row)    -> Double_t
                    { return (row + 0.5)*fPadSizeX - fPadPlaneX/2.; };
  auto calculateY = [this](Double_t peakTb) -> Double_t
                    { return -peakTb*fTBTime*fDriftVelocity/100.; };
  auto calculateZ = [this](Double_t layer)  -> Double_t
                    { return (layer + 0.5)*fPadSizeZ; };

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

    Double_t xPos = calculateX(pad -> GetRow());
    Double_t zPos = calculateZ(pad -> GetLayer());
    Double_t charge = 0;

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

          Double_t yHit = calculateY(tbStart);

          STHit *hit = (STHit *) hitArray -> ConstructedAt(hitNum);
          hit -> Clear();
          hit -> SetHit(hitNum, xPos, yHit, zPos, amplitude);
          hit -> SetRow(pad -> GetRow());
          hit -> SetLayer(pad -> GetLayer());
          hit -> SetTb(tbStart);
          hit -> SetChi2(chi2);
          hit -> SetNDF(fNumTbsCompare);

          hitNum++;

          fPulse -> SetParameter(0, amplitude);
          fPulse -> SetParameter(1, tbStart);

          for (Int_t iTbPulse = 0; iTbPulse < 100; iTbPulse++) {
            Int_t tb = tbStartTemp + iTbPulse;
            adc[tb] -= fPulse -> Eval(tb+0.5);
          }

          iTb = Int_t(tbStart) + 9;
        }

        countRise = 0;
      }
    }

    /*
    for (Int_t iPeak = 0; iPeak < numPeaks; iPeak++) {
      Int_t tbPeak = (Int_t)(ceil((peakFinder.GetPositionX())[iPeak]));

      charge = adc[tbPeak];

      if (fThreshold > 0 && charge < fThreshold)
        continue;

      Double_t tbArray[10] = {0};
      Double_t adcArray[10] = {0};
      
      Int_t countPoints = 0;
      for (Int_t iTb = tbPeak; iTb > tbPeak - 10; iTb--) 
      {
        Double_t adcTemp = adc[iTb];

        //if (adcTemp < charge*fPercPeakMin/100. || adcTemp > charge*fPercPeakMax/100.) // TODO
        if (adcTemp < charge*10./100. || adcTemp > charge*90./100.)
          continue;

        tbArray[countPoints] = iTb;
        adcArray[countPoints] = adcTemp;
        countPoints++;
      }

      // if (countPoints < fMinPointsForFit) // TODO
      if (countPoints < 4) 
        continue;

      Double_t fitConst = 0;
      Double_t fitSlope = 0;
      Double_t chi2 = 0;
      Int_t ndf = 0;

      lslfit(countPoints, tbArray, adcArray, fitConst, fitSlope, chi2);
      Double_t tbHit = -fitConst/fitSlope;
      Double_t yHit = calculateY(tbHit);

      if (yHit > fMaxDriftLength || yHit < -2 * fMaxDriftLength)
        continue;
//      if (yHit > 0 || yHit < -fMaxDriftLength)
//        continue;

      STHit *hit = (STHit *) hitArray -> ConstructedAt(hitNum);
      hit -> Clear();
      hit -> SetHit(hitNum, xPos, yHit, zPos, charge);
      hit -> SetRow(pad -> GetRow());
      hit -> SetLayer(pad -> GetLayer());
      hit -> SetTb(tbHit);
      hit -> SetChi2(chi2);
      hit -> SetNDF(countPoints);

      hitNum++;
    }
    */
  }

#ifdef DEBUG
  LOG(INFO) << "Thread ended!" << FairLogger::endl;
#endif
}

Double_t 
STPSAFastFit::Pulse(Double_t *x, Double_t *par)
{
  if (x[0] < par[1]) 
    return 0;

  Double_t hit_time = x[0] - par[1];
  int hit_time_0p1 = hit_time * 10;
  if (hit_time_0p1 > 1998) 
    return 0;

  Double_t r = 10 * hit_time - hit_time_0p1;
  Double_t val = r * fPulseData[hit_time_0p1 + 1] + (1 - r) * fPulseData[hit_time_0p1];

  return par[0] * val;
}

void 
STPSAFastFit::FitPulse(Double_t *buffer, Double_t tbStart, Double_t &chi2, Double_t &amp)
{
  fPulse -> SetParameter(0, 1);
  fPulse -> SetParameter(1, tbStart);

  Double_t refy = 0;
  Double_t ref2 = 0;

  for (Int_t iTbPulse = 0; iTbPulse < fNumTbsCompare; iTbPulse++) {
    Int_t tb = tbStart + iTbPulse;
    Double_t adc = buffer[tb];

    Double_t ref = fPulse -> Eval(tb+0.5);
    refy += ref * adc;
    ref2 += ref * ref;
  }

  amp = refy / ref2;
  fPulse -> SetParameter(0, amp);

  chi2 = 0;
  for (Int_t iTbPulse = 0; iTbPulse < fNumTbsCompare; iTbPulse++) {
    Int_t tb = tbStart + iTbPulse;
    Double_t adc = buffer[tb];
    Double_t ref = fPulse -> Eval(tb+0.5);
    chi2 += (adc - ref) * (adc - ref);
  }
}
