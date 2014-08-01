// =================================================
//  STPedestal Class
// 
//  Description:
//    This class is used for calculating or finding
//    pedestal value and sigma corresponding to
//    user-input coboIdx, asadIdx, agetIdx & chIdx.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 14
// =================================================

#ifndef STPEDESTAL_H
#define STPEDESTAL_H

#include "TROOT.h"
#include "TObject.h"

#include "TFile.h"
#include "TTree.h"

#include <fstream>

class STPedestal : public TObject {
  public:
    STPedestal();
    STPedestal(TString pedestalData);
    ~STPedestal() {};

    void Initialize();
    Bool_t SetPedestalData(TString pedestalData);

    // Look up pedestal value(0) and sigma(1) array from pedestal run data
//    void GetPedestal(Int_t *samples, Double_t *pedestal, Int_t startBuckIdx = 10, Int_t numSamples = 20);

    // Calculate pedestal value(0) and sigma(1) array from first 20 buckets
    void GetPedestal(Int_t coboIdx, Int_t asadIdx, Int_t agetIdx, Int_t chIdx, Double_t *pedestal, Double_t *pedestalSigma);

  private:
    TFile *fOpenFile;
    TTree *fPedestalTree;

    Int_t fNumTbs;

    Double_t fPedestal[512];
    Double_t fPedestalSigma[512];

  ClassDef(STPedestal, 1);
};

#endif
