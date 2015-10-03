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

// STL
#include <algorithm>

using std::max_element;
using std::min_element;
using std::distance;

ClassImp(STPSASimple)

STPSASimple::STPSASimple()
{
}

STPSASimple::~STPSASimple()
{
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
    Int_t tb = -1;

    if (pad -> IsPedestalSubtracted()) {
      Double_t *adc = pad -> GetADC();
      Int_t maxAdcIdx = distance(adc, max_element(adc + 4, adc + fNumTbs - 5));
      tb = maxAdcIdx;

      yPos = CalculateY(maxAdcIdx);
      charge = adc[maxAdcIdx];

      if (fThreshold > 0 && charge < fThreshold)
        continue;
    } else {
      Int_t *rawAdc = pad -> GetRawADC();
      Int_t minAdcIdx = distance(rawAdc, min_element(rawAdc + 4, rawAdc + fNumTbs - 5));
      tb = minAdcIdx;

      yPos = CalculateY(minAdcIdx);
      charge = rawAdc[minAdcIdx];

      if (fThreshold > 0 && charge > fThreshold)
        continue;
    }


    STHit *hit = new STHit(hitNum, xPos, yPos, zPos, charge);
    hit -> SetRow(pad -> GetRow());
    hit -> SetLayer(pad -> GetLayer());
    hit -> SetTb(tb);
    event -> AddHit(hit);
    delete hit;

    hitNum++;
  }
}
