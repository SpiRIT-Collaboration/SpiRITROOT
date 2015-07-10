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
  fReferenceConstant = -9999;
  fReferenceSlope = -9999;
  fReferenceExponent = 1.;
  fIsSetGainCalibrationData = kFALSE;

  memset(fConstantPol1, 0, sizeof(Double_t)*108*112);
  memset(fSlopePol1, 0, sizeof(Double_t)*108*112);
  memset(fConstantExp, 0, sizeof(Double_t)*108*112);
  memset(fSlopeExp, 0, sizeof(Double_t)*108*112);
  memset(fExponent, 0, sizeof(Double_t)*108*112);

  fDataType = "f";

  memset(fGraph, 0, sizeof(TGraphErrors *)*108*112);
  memset(fGraphR, 0, sizeof(TGraphErrors *)*108*112);

  fReferenceRow = -9999;
  fReferenceLayer = -9999;
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
      Double_t constantPol1 = -9999; 
      Double_t slopePol1 = -9999;
      Double_t constantExp = -9999; 
      Double_t slopeExp = -9999;
      Double_t exponent = 1.;
      fGainCalibrationTree = (TTree *) fOpenFile -> Get("GainCalibrationData");
      fGainCalibrationTree -> SetBranchAddress("padRow", &padRow);
      fGainCalibrationTree -> SetBranchAddress("padLayer", &padLayer);
      fGainCalibrationTree -> SetBranchAddress("constantPol1", &constantPol1);
      fGainCalibrationTree -> SetBranchAddress("slopePol1", &slopePol1);
      fGainCalibrationTree -> SetBranchAddress("constantExp", &constantExp);
      fGainCalibrationTree -> SetBranchAddress("slopeExp", &slopeExp);
      fGainCalibrationTree -> SetBranchAddress("exponent", &exponent);

      Int_t numEntries = fGainCalibrationTree -> GetEntries();
      for (Int_t iEntry = 0; iEntry < numEntries; iEntry++) {
        fGainCalibrationTree -> GetEntry(iEntry);
        
        fConstantPol1[padRow][padLayer] = constantPol1;
        fSlopePol1[padRow][padLayer] = slopePol1;
        fConstantExp[padRow][padLayer] = constantExp;
        fSlopeExp[padRow][padLayer] = slopeExp;
        fExponent[padRow][padLayer] = exponent;
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

void STGainCalibration::SetGainReference(Int_t row, Int_t layer)
{
  fReferenceRow = row;
  fReferenceLayer = layer;
}

void STGainCalibration::SetGainReference(Double_t constant, Double_t slope, Double_t exponent)
{
  fReferenceConstant = constant;
  fReferenceSlope = slope;
  fReferenceExponent = exponent;
}

Bool_t STGainCalibration::IsSetGainCalibrationData()
{
  return fIsSetGainCalibrationData;
}

Bool_t STGainCalibration::CalibrateADC(Int_t padRow, Int_t padLayer, Int_t numTbs, Double_t *adc) {
  if (fIsSetGainCalibrationData == kFALSE) {
    std::cerr << "[STGainCalibration] Gain calibration data is not set!" << std::endl;

    return kFALSE;
  } else if ((fDataType.EqualTo("f") && (fReferenceConstant == -9999 || fReferenceSlope == -9999)) &&
             (fDataType.EqualTo("nf") && (fReferenceRow == -9999 || fReferenceLayer == -9999))) {
    std::cerr << "[STGainCalibration] Gain calibration base is not set!" << std::endl;

    return kFALSE;
  }

  if (fDataType.EqualTo("f")) {
    if (fReferenceExponent == 1.) {
      for (Int_t iTb = 0; iTb < numTbs; iTb++) {
        Double_t newAdc = ((adc[iTb] - fConstantPol1[padRow][padLayer])/fSlopePol1[padRow][padLayer])*fReferenceSlope + fReferenceConstant;
        adc[iTb] = newAdc;
      }
    } else {
      for (Int_t iTb = 0; iTb < numTbs; iTb++) {
        Double_t newAdc = pow((adc[iTb] - fConstantExp[padRow][padLayer])/fSlopeExp[padRow][padLayer], fReferenceExponent/fExponent[padRow][padLayer])*fReferenceSlope + fReferenceConstant;
        adc[iTb] = newAdc;
      }
    }
  } else if (fDataType.EqualTo("nf")) {
    if (padRow == fReferenceRow && padLayer == fReferenceLayer)
      return kTRUE;

    for (Int_t iTb = 0; iTb < numTbs; iTb++) {
      Double_t rawADCtoV = fGraphR[padRow][padLayer] -> Eval(adc[iTb]);
      Double_t VtoCalibADC = fGraph[fReferenceRow][fReferenceLayer] -> Eval(rawADCtoV);
      adc[iTb] = VtoCalibADC;
    }
  }

  return kTRUE;
}
