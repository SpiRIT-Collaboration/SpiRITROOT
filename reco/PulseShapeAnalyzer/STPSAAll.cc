// SpiRITROOT classes
#include "STPSAAll.hh"

// STL
#include <cmath>

// ROOT
#include "RVersion.h"

ClassImp(STPSAAll)

STPSAAll::STPSAAll()
{
  fPeakFinder = new TSpectrum();
}

STPSAAll::~STPSAAll()
{
}

void
STPSAAll::Analyze(STRawEvent *rawEvent, STEvent *event)
{
  Int_t numPads = rawEvent -> GetNumPads();
  Int_t hitNum = 0;

  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad *pad = rawEvent -> GetPad(iPad);

    Int_t layer = pad -> GetLayer();
    if (layer <= fLayerCut)
      continue;

    Double_t xPos = CalculateX(pad -> GetRow());
    Double_t zPos = CalculateZ(layer);
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
      Double_t chi2 = 0;
      Int_t ndf = 0;

      LSLFit(countPoints, tbArray, adcArray, fitConst, fitSlope, chi2);
      Double_t tbHit = -fitConst/fitSlope;
      Double_t yHit = CalculateY(tbHit);

      if (yHit > fMaxDriftLength || yHit < -2 * fMaxDriftLength)
        continue;
//      if (yHit > 0 || yHit < -fMaxDriftLength)
//        continue;

      STHit *hit = new STHit(hitNum, xPos, yHit, zPos, charge);
      hit -> SetRow(pad -> GetRow());
      hit -> SetLayer(pad -> GetLayer());
      hit -> SetTb(tbHit);
      hit -> SetChi2(chi2);
      hit -> SetNDF(countPoints);
      event -> AddHit(hit);
      delete hit;

      hitNum++;
    }
  }
}
