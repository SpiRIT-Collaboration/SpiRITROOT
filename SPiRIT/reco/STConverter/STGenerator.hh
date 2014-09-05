// =================================================
//  STGenerator Class
// 
//  Description:
//    Using the pedestal run data or pulser run data
//    as its input, generates input data for pedestal
//    subtraction or gain calibration data.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 09. 05
// =================================================

#pragma once

#include "TObject.h"
#include "TString.h"

#include "STCore.hh"

#include <vector>

using std::vector;

class STGenerator : public TObject {
  public:
    STGenerator();
    STGenerator(TString mode);
    ~STGenerator();

    void SetMode(TString mode);
    void SetOutputFile(TString filename);
    Bool_t SetParameterDir(TString dir);

    Bool_t AddData(TString filename);
    Bool_t AddData(Double_t voltage, TString filename);
    Bool_t SetData(Int_t index);

    void StartProcess();

    void Print();

  private:
    void GeneratePedestal();
    void GenerateGainCalibrationData();

    Int_t GetIntParameter(TString parameter);
    TString GetFileParameter(Int_t index);

    enum EMode { kError, kPedestal, kGain };
    Int_t fMode;

    vector<Double_t> fVoltageArray;

    STCore *fCore;
    TString fParameterFile;
    TString fOutputFile;

    Int_t fNumTbs;
    Int_t fRows;
    Int_t fLayers;
    Int_t fPadX;
    Int_t fPadZ;

  ClassDef(STGenerator, 1)
};
