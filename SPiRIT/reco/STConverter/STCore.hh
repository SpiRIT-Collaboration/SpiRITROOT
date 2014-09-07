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
#include "STSignalDelay.hh"
#include "GETDecoder.hh"
#include "GETFrame.hh"

class STCore : public TObject {
  public:
    STCore();
    STCore(TString filename);
    STCore(TString filename, Int_t numTbs);
    ~STCore();

    void Initialize();

    // setters
    Bool_t AddGraw(TString filename);
    void SetNoAutoReload(Bool_t value = kFALSE);
    Bool_t SetData(Int_t value);
    Int_t GetNumData();
    TString GetDataName(Int_t index);
    void SetNumTbs(Int_t value);
    void SetInternalPedestal(Int_t startTb = 10, Int_t averageTbs = 20);
    Bool_t SetPedestalData(TString filename, Int_t startTb = 3, Int_t averageTbs = 20);

    Bool_t SetGainCalibrationData(TString filename);
    void SetGainBase(Double_t constant, Double_t slope);

    Bool_t SetSignalDelayData(TString filename);

    Bool_t SetUAMap(TString filename);
    Bool_t SetAGETMap(TString filename);

    // getters
    STRawEvent *GetRawEvent(Int_t eventID = -1);
    Int_t GetNumTbs();

  private:
    STMap *fMapPtr;

    Int_t fNumTbs;

    GETDecoder *fDecoderPtr;
    Bool_t fIsGraw;

    STPedestal *fPedestalPtr;
    Bool_t fIsInternalPedestal;
    Bool_t fIsPedestalData;
    Int_t fStartTb;
    Int_t fAverageTbs;

    STGainCalibration *fGainCalibrationPtr;
    Bool_t fIsGainCalibrationData;

    STSignalDelay *fSignalDelayPtr;
    Bool_t fIsSignalDelayData;

    STRawEvent *fRawEventPtr;

    UInt_t fPrevEventNo;
    UInt_t fCurrEventNo;

    Int_t fCurrFrameNo;

  ClassDef(STCore, 1);
};

#endif
