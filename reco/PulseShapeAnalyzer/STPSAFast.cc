// SpiRITROOT classes
#include "STPSAFast.hh"

ClassImp(STPSAFast)

STPSAFast::STPSAFast()
{
}

STPSAFast::~STPSAFast()
{
}

void
STPSAFast::Analyze(STRawEvent *rawEvent, STEvent *event)
{
  Int_t numPads = rawEvent -> GetNumPads();

  Int_t hitNum = 0;

  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad *pad = rawEvent -> GetPad(iPad);
    Double_t *adc = pad -> GetADC();
    
    Double_t adcMax = 0;
    Int_t idxTbADCMax = 0;

    for (Int_t iTb = 0; iTb < fNumTbs; iTb++) 
    {
      if (adcMax < adc[iTb]) 
      {
        adcMax = adc[iTb];
        idxTbADCMax = iTb;
      }
    }

    if (adcMax < fThreshold)
      continue;

    STHit *hit = new STHit(hitNum, 
                           CalculateX(pad -> GetRow()), 
                           CalculateY(idxTbADCMax),
                           CalculateZ(pad -> GetLayer()),
                           adcMax);
    event -> AddHit(hit);
    delete hit;

    hitNum++;
  }
}
