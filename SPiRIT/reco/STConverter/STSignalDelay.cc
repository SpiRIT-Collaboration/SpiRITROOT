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

#include "STSignalDelay.hh"

#include "TMath.h"
#include "TFile.h"
#include "TTree.h"

#include <iostream>

ClassImp(STSignalDelay);

STSignalDelay::STSignalDelay() {
  Initialize();
}

STSignalDelay::STSignalDelay(TString signalDelayData) {
  Initialize();

  SetSignalDelayData(signalDelayData);
}

void STSignalDelay::Initialize()
{
  fOpenFile = NULL;
  fSignalDelayTree = NULL;

  fIsSetSignalDelayData = kFALSE;
  memset(fSignalDelay, 511, sizeof(Int_t)*4*12);
}

Bool_t STSignalDelay::SetSignalDelayData(TString signalDelayData) {
  if (fOpenFile != NULL)
    delete fOpenFile;

  if ((fOpenFile = new TFile(signalDelayData))) {
    Int_t UAIdx = -2;
    Int_t signalDelay = 511;;

    fSignalDelayTree = (TTree *) fOpenFile -> Get("SignalDelayData");
    fSignalDelayTree -> SetBranchAddress("UAIdx", &UAIdx);
    fSignalDelayTree -> SetBranchAddress("signalDelay", &signalDelay);

    Int_t numEntries = fSignalDelayTree -> GetEntries();
    for (Int_t iEntry = 0; iEntry < numEntries; iEntry++) {
      fSignalDelayTree -> GetEntry(iEntry);

      fSignalDelay[UAIdx/100][UAIdx%100] = signalDelay;
    }

    delete fOpenFile;
    fSignalDelayTree = NULL;
    fOpenFile = NULL;

    fIsSetSignalDelayData = kTRUE;
    return kTRUE;
  }

  return kFALSE;
}

Bool_t STSignalDelay::IsSetSignalDelayData() {
  return fIsSetSignalDelayData;
}

Int_t STSignalDelay::GetSignalDelay(Int_t UAIdx) {
  if (fIsSetSignalDelayData == kFALSE) {
    std::cerr << "SignalDelay data file is not set!" << std::endl;

    return 511;
  }

  return fSignalDelay[UAIdx/100][UAIdx%100];
}
