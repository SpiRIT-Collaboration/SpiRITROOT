// SpiRITROOT classes
#include "STPSAAll.hh"
#include "STGlobal.hh"

// STL
#include <cmath>
#include <thread>

// ROOT
#include "RVersion.h"

//#define DEBUG

ClassImp(STPSAAll)

STPSAAll::STPSAAll()
{
  fPeakFinder = new TSpectrum();

  fThreadHitArray = new TClonesArray*[NUMTHREAD];
  for (Int_t iThread = 0; iThread < NUMTHREAD; iThread++)
    fThreadHitArray[iThread] = new TClonesArray("STHit", 100);

  fPadReady = kFALSE;
  fPadTaken = kFALSE;
  fEnd = kFALSE;
}

void
STPSAAll::Analyze(STRawEvent *rawEvent, STEvent *event)
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

    if (fPad -> GetLayer() <= fLayerLowCut) {
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

void STPSAAll::PadAnalyzer(TClonesArray *hitArray)
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
  auto lslfit = [this](Int_t numPoints, Double_t *x, Double_t *y, Double_t &constant, Double_t &slope, Double_t &chi2)
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
                    };

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

    Double_t dummy[512] = {0};

    Int_t numPeaks = peakFinder.SearchHighRes(adc, dummy, fNumTbs, 4.7, 5, kFALSE, 3, kTRUE, 3);

    if (numPeaks == 0)
      continue;

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
  }

#ifdef DEBUG
  LOG(INFO) << "Thread ended!" << FairLogger::endl;
#endif
}
