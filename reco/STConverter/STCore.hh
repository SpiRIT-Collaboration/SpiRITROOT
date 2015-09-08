// =================================================
//  STCore Class
// 
//  Description:
//    Process CoBoFrame data into STRawEvent data
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 09. 25
// =================================================

#ifndef _STCORE_H_
#define _STCORE_H_

#include "TObject.h"
#include "TString.h"

#include "STRawEvent.hh"
#include "STMap.hh"
#include "STPedestal.hh"
#include "STGainCalibration.hh"
#include "STPlot.hh"

#include "GETDecoder.hh"
#include "GETFrame.hh"

class STPlot;

class STCore : public TObject {
  public:
    STCore();
    STCore(TString filename);
    STCore(TString filename, Int_t numTbs, Int_t windowNumTbs = 512, Int_t windowStartTb = 0);
    ~STCore();

    void Initialize();

    // setters
    Bool_t AddData(TString filename);
    void SetNoAutoReload(Bool_t value = kFALSE);
    void SetPedestalGenerationMode(Bool_t value = kTRUE);
    void SetPositivePolarity(Bool_t value = kTRUE);
    Bool_t SetData(Int_t value);
    Int_t GetNumData();
    TString GetDataName(Int_t index);
    void SetNumTbs(Int_t value);
    void SetWindow(Int_t numTbs, Int_t startTb);
    void SetInternalPedestal(Int_t pedestalStartTb = 10, Int_t averageTbs = 20);
    Bool_t SetPedestalData(TString filename, Double_t rmsFactor = 0);
    void SetFPNPedestal(Double_t sigmaThreshold = 5);

    Bool_t SetGainCalibrationData(TString filename, TString dataType = "f");
    void SetGainReference(Int_t row, Int_t layer);
    void SetGainReference(Double_t constant, Double_t linear, Double_t quadratic = 0.);

    Bool_t SetUAMap(TString filename);
    Bool_t SetAGETMap(TString filename);

    void SetOldData(Bool_t oldData = kTRUE);

    // getters
    STRawEvent *GetRawEvent(Int_t eventID = -1);
    Int_t GetNumTbs();

    STMap *GetSTMap();
    STPlot *GetSTPlot();

    enum EPedestalMode { kNoPedestal, kPedestalInternal, kPedestalExternal, kPedestalFPN, kPedestalBothIE };

  private:
    STMap *fMapPtr;
    STPlot *fPlotPtr;

    Int_t fNumTbs;

    Int_t fWindowNumTbs;
    Int_t fWindowStartTb;

    GETDecoder *fDecoderPtr;
    Bool_t fIsData;

    STPedestal *fPedestalPtr;
    Bool_t fIsPedestalGenerationMode;
    Bool_t fIsInternalPedestal;
    Bool_t fIsPedestalData;
    Bool_t fIsFPNPedestal;
    Double_t fFPNSigmaThreshold;
    EPedestalMode fPedestalMode;
    Double_t fPedestalRMSFactor;
    Int_t fPedestalStartTb;
    Int_t fAverageTbs;

    STGainCalibration *fGainCalibrationPtr;
    Bool_t fIsGainCalibrationData;

    STRawEvent *fRawEventPtr;

    UInt_t fPrevEventNo;
    UInt_t fCurrEventNo;

    Int_t fCurrFrameNo;

    Bool_t fOldData;

  ClassDef(STCore, 1);
};

#endif
