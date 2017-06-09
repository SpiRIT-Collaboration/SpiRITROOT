// =================================================
//  STGainMatching Class
// 
//  Description:
//    Developed for matching gain of anode 12 and 14
//    which have the low anode voltage.
//    However, the class is designed to be used for
//    the other anode parts.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2017. 01. 20
// =================================================

#include "STGainMatching.hh"

ClassImp(STGainMatching)

STGainMatching::STGainMatching() {
  ClassInit();
}

STGainMatching::STGainMatching(TString datafile) {
  ClassInit();

  SetDatafile(datafile);
}

Bool_t STGainMatching::Init() {
  if (fDatafile.EqualTo("")) {
    std::cout << "= [STGainMatching] Data file is not set specified!" << std::endl;

    fIsInitialized = kFALSE;
    return fIsInitialized;
  }

  Int_t layer;
  Double_t relativeGain;
  std::ifstream matchList(fDatafile.Data());
  for (Int_t iLayer = 0; iLayer < 112; iLayer++) {
    matchList >> layer >> relativeGain;
    fRelativeGain[layer] = relativeGain;
  }

  fIsInitialized = kTRUE;
  return fIsInitialized;
}

Bool_t STGainMatching::CalibrateADC(Int_t layer, Int_t numTbs, Double_t *adc) {
  if (!fIsInitialized) {
    std::cout << "= [STGainMatching] Class is not initialized properly!" << std::endl;

    return fIsInitialized;
  }

  for (Int_t iTb = 0; iTb < numTbs; iTb++)
    adc[iTb] = adc[iTb]*fRelativeGain[layer];

  return fIsInitialized;
}

void STGainMatching::ClassInit() {
  fDatafile = "";
  memset(fRelativeGain, 1.0, sizeof(Double_t)*112);
}

void STGainMatching::SetDatafile(TString datafile) {
  fDatafile = datafile;
}
