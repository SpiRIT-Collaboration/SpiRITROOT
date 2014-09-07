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
  fBaseConstant = -9999;
  fBaseSlope = -9999;
  fIsSetGainCalibrationData = kFALSE;

  memset(fConstant, 0, sizeof(Double_t)*108*112);;
  memset(fSlope, 0, sizeof(Double_t)*108*112);;
}

Bool_t STGainCalibration::SetGainCalibrationData(TString gainCalibrationData) {
  if (fOpenFile != NULL)
    delete fOpenFile;

  if ((fOpenFile = new TFile(gainCalibrationData))) {
    Int_t padRow = -2; 
    Int_t padLayer = -2; 
    Double_t constant = -9999; 
    Double_t slope = -9999;
    fGainCalibrationTree = (TTree *) fOpenFile -> Get("GainCalibrationData");
    fGainCalibrationTree -> SetBranchAddress("padRow", &padRow);
    fGainCalibrationTree -> SetBranchAddress("padLayer", &padLayer);
    fGainCalibrationTree -> SetBranchAddress("constant", &constant);
    fGainCalibrationTree -> SetBranchAddress("slope", &slope);

    Int_t numEntries = fGainCalibrationTree -> GetEntries();
    for (Int_t iEntry = 0; iEntry < numEntries; iEntry++) {
      fGainCalibrationTree -> GetEntry(iEntry);
      
      fConstant[padRow][padLayer] = constant;
      fSlope[padRow][padLayer] = slope;
    }

    delete fOpenFile;
    fGainCalibrationTree = NULL;
    fOpenFile = NULL;

    fIsSetGainCalibrationData = kTRUE;
    return kTRUE;
  }

  return kFALSE;
}

Bool_t STGainCalibration::SetGainBase(Double_t constant, Double_t slope)
{
  fBaseConstant = constant;
  fBaseSlope = slope;
}

Bool_t STGainCalibration::IsSetGainCalibrationData()
{
  return fIsSetGainCalibrationData;
}

Bool_t STGainCalibration::CalibrateADC(Int_t padRow, Int_t padLayer, Int_t numTbs, Double_t *adc) {
  if (fIsSetGainCalibrationData == kFALSE) {
    std::cerr << "[STGainCalibration] Gain calibration data is not set!" << std::endl;

    return kFALSE;
  } else if (fBaseConstant == -9999 || fBaseSlope == -9999) {
    std::cerr << "[STGainCalibration] Gain calibration base is not set!" << std::endl;

    return kFALSE;
  }

  for (Int_t iTb = 0; iTb < numTbs; iTb++) {
    Double_t newAdc = ((adc[iTb] - fConstant[padRow][padLayer])/fSlope[padRow][padLayer])*fBaseSlope + fBaseConstant;
    adc[iTb] = (newAdc > 0 ? newAdc : 0);
  }

  return kTRUE;
}
