// =================================================
//  STGainColaboration Class
// 
//  Description:
//    This class provides gain scale factor.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 08. 25
// =================================================

#include "STGainCalibration.hh"

#include "TMath.h"
#include "TFile.h"
#include "TTree.h"

#include <iostream>

ClassImp(STGainCalibration);

STGainCalibration::STGainCalibration() {
  Initialize();
}

STGainCalibration::STGainCalibration(TString gainCalibrationData) {
  Initialize();

  SetGainCalibrationData(gainCalibrationData);
}

void STGainCalibration::Initialize()
{
  fOpenFile = NULL;
  fGainCalibrationTree = NULL;
  fIsSetGainCalibrationData = kFALSE;

  memset(fScaleFactor, 0, sizeof(Double_t)*108*112);;
}

Bool_t STGainCalibration::SetGainCalibrationData(TString gainCalibrationData) {
  if (fOpenFile != NULL)
    delete fOpenFile;

  if ((fOpenFile = new TFile(gainCalibrationData))) {
    Int_t padRow = -2; 
    Int_t padLayer = -2; 
    Double_t scaleFactor = 1; 
    fGainCalibrationTree = (TTree *) fOpenFile -> Get("GainCalibrationData");
    fGainCalibrationTree -> SetBranchAddress("padRow", &padRow);
    fGainCalibrationTree -> SetBranchAddress("padLayer", &padLayer);
    fGainCalibrationTree -> SetBranchAddress("scaleFactor", &scaleFactor);

    Int_t numEntries = fGainCalibrationTree -> GetEntries();
    for (Int_t iEntry = 0; iEntry < numEntries; iEntry++) {
      fGainCalibrationTree -> GetEntry(iEntry);
      
      fScaleFactor[padRow][padLayer] = scaleFactor;
    }

    delete fOpenFile;
    fGainCalibrationTree = NULL;
    fOpenFile = NULL;

    fIsSetGainCalibrationData = kTRUE;
    return kTRUE;
  }

  return kFALSE;
}

Bool_t STGainCalibration::IsSetGainCalibrationData()
{
  return fIsSetGainCalibrationData;
}

Double_t STGainCalibration::GetScaleFactor(Int_t padRow, Int_t padLayer) {
  if (fIsSetGainCalibrationData == kFALSE) {
    std::cerr << "[STGainCalibration::GetScaleFactor()] Gain calibration data is not set!" << std::endl;

    return -1;
  }

  return fScaleFactor[padRow][padLayer];
}
