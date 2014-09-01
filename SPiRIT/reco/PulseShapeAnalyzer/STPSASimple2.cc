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

// FairRoot classes
#include "FairRuntimeDb.h"
#include "FairRun.h"

// STL
#include <algorithm>
#include <cmath>

ClassImp(STPSASimple2)

STPSASimple2::STPSASimple2()
{
  fLogger = FairLogger::GetLogger();

  FairRun *run = FairRun::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "STDigiPar not found!!");

  fPadPlaneX = fPar -> GetPadPlaneX();
  fPadSizeX = fPar -> GetPadSizeX();
  fPadSizeZ = fPar -> GetPadSizeZ();

  fNumTbs = fPar -> GetNumTbs();
  fTBTime = fPar -> GetTBTime();
  fDriftVelocity = fPar -> GetDriftVelocity();

  fThreshold = -1;

  fPeakFinder = new TSpectrum();
}

STPSASimple2::~STPSASimple2()
{
}

void
STPSASimple2::SetThreshold(Int_t threshold)
{
  fThreshold = threshold;
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
    Double_t yPos = 0;
    Double_t charge = 0;

    if (!(pad -> IsPedestalSubtracted())) {
      fLogger -> Error(MESSAGE_ORIGIN, "Pedestal should be subtracted to use this class!");

      return;
    }

    Double_t *adc = pad -> GetADC();
    Float_t floatADC[512] = {0};
    Float_t dummy[512] = {0};
    for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
      floatADC[iTb] = adc[iTb];

    Int_t numPeaks = fPeakFinder -> SearchHighRes(floatADC, dummy, fNumTbs, 4.7, 5, kFALSE, 3, kTRUE, 3);

    if (numPeaks == 0)
      continue;

    for (Int_t iPeak = 0; iPeak < numPeaks; iPeak++) {
      Int_t maxAdcIdx = (Int_t)(ceil((fPeakFinder -> GetPositionX())[iPeak]));

      yPos = CalculateY(maxAdcIdx);
      charge = adc[maxAdcIdx];

      if (fThreshold > 0 && charge < fThreshold)
        continue;

      STHit *hit = new STHit(hitNum, xPos, yPos, zPos, charge);
      event -> AddHit(hit);
      delete hit;

      hitNum++;
    }
  }
}

Double_t
STPSASimple2::CalculateX(Int_t row)
{
  return (row + 0.5)*fPadSizeX - fPadPlaneX/2.;
}

Double_t
STPSASimple2::CalculateY(Int_t peakIdx)
{
  return -peakIdx*fTBTime*fDriftVelocity/100.;
}

Double_t
STPSASimple2::CalculateZ(Int_t layer)
{
  return (layer + 0.5)*fPadSizeZ;
}
