// =================================================
//  STSignalDelay Class
// 
//  Description:
//    This class is used for calculating or finding
//    signal delay value corresponding to user-input
//    padRow and padLayer
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 08. 26
// =================================================

#ifndef STSIGNALDELAY_H
#define STSIGNALDELAY_H

#include "TROOT.h"
#include "TObject.h"

#include "TFile.h"
#include "TTree.h"

#include <fstream>

class STSignalDelay : public TObject {
  public:
    STSignalDelay();
    STSignalDelay(TString signalDelayData);
    ~STSignalDelay() {};

    void Initialize();
    Bool_t SetSignalDelayData(TString signalDelayData);

    // Look up signalDelay value(0) and sigma(1) array from signalDelay run data
//    void GetSignalDelay(Int_t *samples, Double_t *signalDelay, Int_t startBuckIdx = 10, Int_t numSamples = 20);

    Bool_t IsSetSignalDelayData();
    Int_t GetSignalDelay(Int_t UAIdx);

  private:
    TFile *fOpenFile;
    TTree *fSignalDelayTree;

    Bool_t fIsSetSignalDelayData;

    Int_t fSignalDelay[4][12];

  ClassDef(STSignalDelay, 1);
};

#endif
