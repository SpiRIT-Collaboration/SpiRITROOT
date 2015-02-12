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

STGainCalibration::STGainCalibration(TString gainCalibrationData, TString dataType) {
  Initialize();

  SetGainCalibrationData(gainCalibrationData, dataType);
}

void STGainCalibration::Initialize()
{
  fOpenFile = NULL;
  fGainCalibrationTree = NULL;
  fBaseConstant = -9999;
  fBaseSlope = -9999;
  fIsSetGainCalibrationData = kFALSE;

  memset(fConstant, 0, sizeof(Double_t)*108*112);
  memset(fSlope, 0, sizeof(Double_t)*108*112);

  fDataType = "f";

  memset(fGraph, 0, sizeof(TGraphErrors *)*108*112);
  memset(fGraphR, 0, sizeof(TGraphErrors *)*108*112);

  fBaseRow = -9999;
  fBaseLayer = -9999;
}

Bool_t STGainCalibration::SetGainCalibrationData(TString gainCalibrationData, TString dataType) {
  if (fOpenFile != NULL)
    delete fOpenFile;

  dataType.ToLower();
  fDataType = dataType;

  if ((fOpenFile = new TFile(gainCalibrationData))) {
    if (fDataType.EqualTo("f")) {
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
    } else if (fDataType.EqualTo("nf")) {
      for (Int_t iRow = 0; iRow < 108; iRow++) {
        for (Int_t iLayer = 0; iLayer < 112; iLayer++) {
          fGraph[iRow][iLayer] = (TGraphErrors *) fOpenFile -> Get(Form("pad_%d_%d", iRow, iLayer));
          fGraphR[iRow][iLayer] = (TGraphErrors *) fOpenFile -> Get(Form("pad_%d_%d_r", iRow, iLayer));
        }
      }

      fIsSetGainCalibrationData = kTRUE;
      return kTRUE;
    }
  }

  return kFALSE;
}

void STGainCalibration::SetGainBase(Int_t row, Int_t layer)
{
  fBaseRow = row;
  fBaseLayer = layer;
}

void STGainCalibration::SetGainBase(Double_t constant, Double_t slope)
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
  } else if ((fDataType.EqualTo("f") && (fBaseConstant == -9999 || fBaseSlope == -9999)) &&
             (fDataType.EqualTo("nf") && (fBaseRow == -9999 || fBaseLayer == -9999))) {
    std::cerr << "[STGainCalibration] Gain calibration base is not set!" << std::endl;

    return kFALSE;
  }

  if (fDataType.EqualTo("f")) {
    for (Int_t iTb = 0; iTb < numTbs; iTb++) {
      Double_t newAdc = ((adc[iTb] - fConstant[padRow][padLayer])/fSlope[padRow][padLayer])*fBaseSlope + fBaseConstant;
//      adc[iTb] = (newAdc > 0 ? newAdc : 0);
      adc[iTb] = newAdc;
    }
  } else if (fDataType.EqualTo("nf")) {
    if (padRow == fBaseRow && padLayer == fBaseLayer)
      return kTRUE;

    for (Int_t iTb = 0; iTb < numTbs; iTb++) {
      Double_t rawADCtoV = fGraphR[padRow][padLayer] -> Eval(adc[iTb]);
      Double_t VtoCalibADC = fGraph[fBaseRow][fBaseLayer] -> Eval(rawADCtoV);
      adc[iTb] = VtoCalibADC;
    }
  }

  return kTRUE;
}
