// =================================================
//  STGainCheck Class
// 
//  Description:
//    Test gain using processed pulser data
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2014. 09. 07
// =================================================

#pragma once

#include "TObject.h"
#include "TString.h"
#include "TCanvas.h"
#include "TH2D.h"

#include "STCore.hh"

#include <vector>

using std::vector;

class STGainCheck : public TObject
{
  public:
    STGainCheck();
    ~STGainCheck();

    void SetUAIndex(Int_t uaIdx);
    void SetGainReference(Double_t constant, Double_t linear, Double_t quadratic);
    void SetGainReference(Int_t row, Int_t layer);
    Bool_t AddPulserData(TString filename);
    void SetInternalPedestal(Int_t startTb, Int_t numTbs);
    void SetFPNPedestal(Double_t sigmaThreshold = 5);
    Bool_t SetPedestalData(TString filename);
    void SetGainCalibrationData(TString filename, TString dataType = "f");

    TCanvas *DrawSpectra();

    Bool_t SetParameterDir(TString dir);

  private:
    Int_t GetIntParameter(TString parameter);
    TString GetFileParameter(Int_t index);

    Int_t GetPadIndex(Int_t row, Int_t layer);

    TCanvas *fCanvas;
    TH2D *fSpectra;

    Bool_t fIsGainCalibrationData;

    STCore *fCore;

    Int_t fUAIdx;
    Int_t fMinRow;
    Int_t fMaxRow;
    Int_t fMinLayer;
    Int_t fMaxLayer;
    TString fPadName[252];

    TString fParameterFile;
    Int_t fNumTbs;
    Int_t fRows;
    Int_t fLayers;
    Int_t fPadX;
    Int_t fPadZ;

  ClassDef(STGainCheck, 1)
};
