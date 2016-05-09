// SpiRITROOT classes
#include "STPSADF.hh"

// ROOT
#include "RVersion.h"
#include <iostream>

ClassImp(STPSADF)

STPSADF::STPSADF()
{
  fMaxFitPoints = 10;
  fMinFitPoints = 4;
}

STPSADF::~STPSADF()
{
}

void
STPSADF::Analyze(STRawEvent *rawEvent, STEvent *event)
{
  Int_t numPads = rawEvent -> GetNumPads();
  Int_t hitNum = 0;

  fBinCandidates.clear();

  for (Int_t iPad = 0; iPad < numPads; iPad++) 
  {
    STPad *pad = rawEvent -> GetPad(iPad);

    Int_t layer = pad -> GetLayer();
    if (layer <= fLayerLowCut)
      continue;

    Double_t xPos = CalculateX(pad -> GetRow());
    Double_t zPos = CalculateZ(layer);

    if (!(pad -> IsPedestalSubtracted())) {
      fLogger -> Error(MESSAGE_ORIGIN, "Pedestal should be subtracted to use this class!");

      return;
    }

    Double_t *adc = pad -> GetADC();

    Double_t binBufferFit[10] = {0};
    Double_t valBufferFit[10] = {0};

    Double_t adcF = 0;
    Double_t adcI = 0;
    Double_t adcMax = 0;
    for (Int_t iTb=0; iTb<fNumTbs; iTb++) 
    {
      adcF = adc[iTb];
      Double_t adcDiff = adcF - adcI;
      adcI = adcF;

      if (adcDiff > 5) 
      //if (adcDiff > fThreshold) 
      {
        fBinCandidates.push_back(iTb);
        adcMax = adcF;
      }
      else
      {
        Int_t numCands = fBinCandidates.size();
        if (numCands >= fMinFitPoints) 
        {
          if (numCands > fMaxFitPoints) 
            numCands = fMaxFitPoints;

          //std::cout << "  pad, tb " << iPad << " " << iTb << " " << numCands << std::endl;

          Int_t countPoints = 0;
          for (Int_t iCand = 0; iCand < numCands; iCand++)
          {
            Int_t bin = fBinCandidates.back();
            fBinCandidates.pop_back();
            Double_t adcCand = adc[bin];
            //std::cout << "      " << iCand << " " << bin << " " << adcCand << " " << std::endl;
            if (adcCand < adcMax * 0.1 || adcCand > adcMax * 0.9)
              continue;
            binBufferFit[countPoints] = bin;
            valBufferFit[countPoints] = adcCand;
            countPoints++;
          }
          fBinCandidates.clear();
          //std::cout << "  count point : " << countPoints << std::endl;
          if (countPoints < fMinFitPoints)
            continue;

          Double_t fitConst = 0;
          Double_t fitSlope = 0;
          Double_t chi2 = 0;

          LSLFit(countPoints, binBufferFit, valBufferFit, fitConst, fitSlope, chi2);
          Double_t tbHit = -fitConst/fitSlope;
          Double_t yHit = CalculateY(tbHit);

          if (yHit > 0 || yHit < -fMaxDriftLength)
            continue;

          if (yHit > 0 || yHit < -fMaxDriftLength)
            continue;

          //std::cout << "    z, y " << zPos << " " << adcMax << std::endl;
          STHit *hit = new STHit(hitNum, xPos, yHit, zPos, adcMax);
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
  }
}
