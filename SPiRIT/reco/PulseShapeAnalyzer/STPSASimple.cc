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
#include "STPSASimple.hh"

// FairRoot classes
#include "FairRuntimeDb.h"
#include "FairRun.h"

// STL
#include <algorithm>

using std::max_element;
using std::min_element;
using std::distance;

ClassImp(STPSASimple)

STPSASimple::STPSASimple()
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
}

STPSASimple::~STPSASimple()
{
}

void
STPSASimple::SetThreshold(Int_t threshold)
{
  fThreshold = threshold;
}

void
STPSASimple::Analyze(STRawEvent *rawEvent, STEvent *event)
{
  Int_t numPads = rawEvent -> GetNumPads();
  Int_t hitNum = 0;

  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad *pad = rawEvent -> GetPad(iPad);
    
    Double_t xPos = CalculateX(pad -> GetRow());
    Double_t zPos = CalculateZ(pad -> GetLayer());
    Double_t yPos = 0;
    Double_t charge = 0;

    if (pad -> IsPedestalSubtracted()) {
      Double_t *adc = pad -> GetADC();
      Int_t maxAdcIdx = distance(adc, max_element(adc + 4, adc + fNumTbs - 5));

      yPos = CalculateY(maxAdcIdx);
      charge = adc[maxAdcIdx];

      if (fThreshold > 0 && charge < fThreshold)
        continue;
    } else {
      Int_t *rawAdc = pad -> GetRawADC();
      Int_t minAdcIdx = distance(rawAdc, min_element(rawAdc + 4, rawAdc + fNumTbs - 5));

      yPos = CalculateY(minAdcIdx);
      charge = rawAdc[minAdcIdx];

      if (fThreshold > 0 && charge > fThreshold)
        continue;
    }


    STHit *hit = new STHit(hitNum, xPos, yPos, zPos, charge);
    event -> AddHit(hit);
    delete hit;

    hitNum++;
  }
}

Double_t
STPSASimple::CalculateX(Int_t row)
{
  return (row + 0.5)*fPadSizeX - fPadPlaneX/2.;
}

Double_t
STPSASimple::CalculateY(Int_t peakIdx)
{
  return -peakIdx*fTBTime*fDriftVelocity/100.;
}

Double_t
STPSASimple::CalculateZ(Int_t layer)
{
  return (layer + 0.5)*fPadSizeZ;
}
