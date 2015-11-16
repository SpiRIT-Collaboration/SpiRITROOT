// =================================================
//  STPedestal Class
// 
//  Description:
//    This class is used for calculating or finding
//    pedestal value and sigma corresponding to
//    user-input padRow and padLayer.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 14
// =================================================

#include "STPedestal.hh"

#include <iostream>

ClassImp(STPedestal);

STPedestal::STPedestal() {
  fMath = new GETMath();
}

Bool_t STPedestal::SubtractPedestal(   Int_t  numTbs,
                                       Int_t *fpn,
                                       Int_t *rawADC,
                                    Double_t *dest,
                                    Double_t  rmsCut,
                                      Bool_t  signalNegativePolarity,
                                       Int_t  startTb,
                                       Int_t  averageTbs
                                   )
{
  while (1) {
    fMath -> Reset();
    for (Int_t iTb = startTb; iTb < startTb + averageTbs; iTb++)
      fMath -> Add(rawADC[iTb]);

    if (fMath -> GetRMS() < rmsCut)
      break;

    startTb += averageTbs;

    if (startTb > numTbs - averageTbs - 3) {
      std::cout << "= [STPedestal] There's no part satisfying sigma threshold " << rmsCut << "!" << std::endl;

      return kFALSE;
    }
  }

  Double_t baselineDiff = -fMath -> GetMean();

  fMath -> Reset();
  for (Int_t iTb = startTb; iTb < startTb + averageTbs; iTb++)
    fMath -> Add(fpn[iTb]);

  baselineDiff += fMath -> GetMean();

  for (Int_t iTb = 0; iTb < numTbs; iTb++) {
    Double_t adc = 0;
    if (signalNegativePolarity == kTRUE)
      adc = (fpn[iTb] - baselineDiff) - rawADC[iTb];
    else
      adc = rawADC[iTb] - (fpn[iTb] - baselineDiff);

    dest[iTb] = adc;
  }

  return kTRUE;
}
