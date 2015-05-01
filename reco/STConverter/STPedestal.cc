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

#include "TMath.h"
#include "TFile.h"
#include "TTree.h"

#include <iostream>

ClassImp(STPedestal);

STPedestal::STPedestal() {
  Initialize();
}

STPedestal::STPedestal(TString pedestalData) {
  Initialize();

  SetPedestalData(pedestalData);
}

void STPedestal::Initialize()
{
  fOpenFile = NULL;
  fPedestalTree = NULL;

  fIsSetPedestalData = kFALSE;

  memset(fPedestal, 0, sizeof(Double_t)*108*112*512);;
  memset(fPedestalSigma, 0, sizeof(Double_t)*108*112*512);;
}

Bool_t STPedestal::SetPedestalData(TString pedestalData) {
  if (fOpenFile != NULL)
    delete fOpenFile;

  if ((fOpenFile = new TFile(pedestalData))) {
    Int_t padRow = -2;
    Int_t padLayer = -2;
    Double_t pedestal[512] = {0};
    Double_t pedestalSigma[512] = {0};

    fPedestalTree = (TTree *) fOpenFile -> Get("PedestalData");
    fPedestalTree -> SetBranchAddress("padRow", &padRow);
    fPedestalTree -> SetBranchAddress("padLayer", &padLayer);
    fPedestalTree -> SetBranchAddress("pedestal", pedestal);
    fPedestalTree -> SetBranchAddress("pedestalSigma", pedestalSigma);

    Int_t numEntries = fPedestalTree -> GetEntries();
    for (Int_t iEntry = 0; iEntry < numEntries; iEntry++) {
      fPedestalTree -> GetEntry(iEntry);

      memcpy(pedestal, fPedestal[padRow][padLayer], sizeof(pedestal));
      memcpy(pedestalSigma, fPedestalSigma[padRow][padLayer], sizeof(pedestalSigma));
    }

    delete fOpenFile;
    fPedestalTree = NULL;
    fOpenFile = NULL;

    fIsSetPedestalData = kTRUE;
    return kTRUE;
  }

  return kFALSE;
}

Bool_t STPedestal::IsSetPedestalData() {
  return fIsSetPedestalData;
}

void STPedestal::GetPedestal(Int_t padRow, Int_t padLayer, Double_t *pedestal, Double_t *pedestalSigma) {
  if (fIsSetPedestalData == kFALSE) {
    std::cerr << "Pedestal data file is not set!" << std::endl;

    memset(pedestal, 4096, sizeof(Double_t)*512);
    memset(pedestalSigma, 4096, sizeof(Double_t)*512);
    return;
  }

  memcpy(pedestal, fPedestal[padRow][padLayer], sizeof(Double_t)*512);
  memcpy(pedestalSigma, fPedestalSigma[padRow][padLayer], sizeof(Double_t)*512);
}
