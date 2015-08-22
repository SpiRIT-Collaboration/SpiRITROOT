//-----------------------------------------------------------
// Description:
//   Simple version of analyzing pulse shape of raw signal.
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STPSASimple2.hh"

// STL
#include <cmath>

// ROOT
#include "RVersion.h"

ClassImp(STPSASimple2)

STPSASimple2::STPSASimple2()
{
  fPeakFinder = new TSpectrum();
}

STPSASimple2::~STPSASimple2()
{
}

void
STPSASimple2::Analyze(STRawEvent *rawEvent, STEvent *event)
{
  Int_t numPads = rawEvent -> GetNumPads();
  Int_t hitNum = 0;

  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad *pad = rawEvent -> GetPad(iPad);

    Double_t xPos = CalculateX(pad -> GetRow());
    Double_t zPos = CalculateZ(pad -> GetLayer());
    Double_t charge = 0;

    if (!(pad -> IsPedestalSubtracted())) {
      fLogger -> Error(MESSAGE_ORIGIN, "Pedestal should be subtracted to use this class!");

      return;
    }

    Double_t *adc = pad -> GetADC();

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

    for (Int_t iPeak = 0; iPeak < numPeaks; iPeak++) {
      Int_t tbPeak = (Int_t)(ceil((fPeakFinder -> GetPositionX())[iPeak]));

      charge = adc[tbPeak];

      if (fThreshold > 0 && charge < fThreshold)
        continue;


      Double_t tbArray[10] = {0};
      Double_t adcArray[10] = {0};
      
      Int_t countPoints = 0;
      for(Int_t iTb=tbPeak; iTb>tbPeak-10; iTb--) 
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
      LSLFit(countPoints, tbArray, adcArray, fitConst, fitSlope);
      Double_t tbHit = -fitConst/fitSlope;
      Double_t yHit = CalculateY(tbHit);
      if (yHit > 0 || yHit < -fMaxDriftLength)
        continue;

      if (yHit > 0 || yHit < -fMaxDriftLength)
        continue;

      STHit *hit = new STHit(hitNum, xPos, yHit, zPos, charge);
      event -> AddHit(hit);
      delete hit;

      hitNum++;
    }
  }
}

void
STPSASimple2::LSLFit(Int_t numPoints, Double_t *x, Double_t *y, Double_t &constant, Double_t &slope)
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
