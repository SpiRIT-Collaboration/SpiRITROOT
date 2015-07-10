// =================================================
//  STGainCalibration Class
// 
//  Description:
//    This class provides gain scale factor.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 08. 25
// =================================================

#ifndef STGAINCALIBRATION_H
#define STGAINCALIBRATION_H

#include "TROOT.h"
#include "TObject.h"

#include "TFile.h"
#include "TTree.h"
#include "TGraphErrors.h"

#include <fstream>

class STGainCalibration : public TObject {
  public:
    STGainCalibration();
    STGainCalibration(TString gainCalibrationData, TString dataType = "f");
    ~STGainCalibration() {};

    void Initialize();
    Bool_t SetGainCalibrationData(TString gainCalibrationData, TString dataType);

    Bool_t IsSetGainCalibrationData();
    void SetGainReference(Int_t row, Int_t layer);
    void SetGainReference(Double_t constant, Double_t slope, Double_t exponent = 1.);
    Bool_t CalibrateADC(Int_t padRow, Int_t padLayer, Int_t numTbs, Double_t *adc);

  private:
    TFile *fOpenFile;
    TTree *fGainCalibrationTree;

    Bool_t fIsSetGainCalibrationData;

    Double_t fConstantPol1[108][112];
    Double_t fSlopePol1[108][112];
    Double_t fConstantExp[108][112];
    Double_t fSlopeExp[108][112];
    Double_t fExponent[108][112];

    Double_t fReferenceConstant;
    Double_t fReferenceSlope;
    Double_t fReferenceExponent;

    TString fDataType;

    TGraphErrors *fGraph[108][112];
    TGraphErrors *fGraphR[108][112];

    Int_t fReferenceRow;
    Int_t fReferenceLayer;

  ClassDef(STGainCalibration, 1);
};

#endif
