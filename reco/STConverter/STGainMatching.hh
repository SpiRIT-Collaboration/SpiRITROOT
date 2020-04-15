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

#ifndef STGAINMATCHING
#define STGAINMATCHING

#include "TROOT.h"
#include "TString.h"

#include <fstream>
#include <iostream>

class STGainMatching {
  public:
    STGainMatching();
    STGainMatching(TString datafile);
    ~STGainMatching() {};

    Bool_t Init();
    void SetDatafile(TString datafile);
    Bool_t CalibrateADC(Int_t layer, Int_t row, Int_t numTbs, Double_t *adc);

  private:
    void ClassInit();

    Bool_t fIsInitialized;
    TString fDatafile;
    Double_t fRelativeGain[112][108];

  ClassDef(STGainMatching, 1)
};

#endif
