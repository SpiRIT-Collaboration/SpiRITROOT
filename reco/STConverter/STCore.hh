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
#include "TClonesArray.h"

#include "STRawEvent.hh"
#include "STMap.hh"
#include "STPedestal.hh"
#include "STGainCalibration.hh"
#include "STGainMatching.hh"
#include "STGGNoiseSubtractor.hh"
#include "STPlot.hh"

#include "GETDecoder.hh"

#include <tuple>

class STPlot;

class STCore : public TObject {
  public:
    STCore();
    STCore(TString filename);
    STCore(TString filename, Int_t numTbs, Int_t windowNumTbs = 512, Int_t windowStartTb = 0);

    void Initialize();

    Bool_t AddData(TString filename, Int_t coboIdx = 0);
    void SetPositivePolarity(Bool_t value = kTRUE);
    Bool_t SetData(Int_t value);
    void SetDiscontinuousData(Bool_t value = kTRUE);
    Int_t GetNumData(Int_t coboIdx = 0);
    TString GetDataName(Int_t index, Int_t coboIdx = 0);
    void SetNumTbs(Int_t value);
    void SetFPNPedestal(Double_t sigmaThreshold = 5);

    void SetGGNoiseGenerationMode(Bool_t value = kTRUE);
    void SetGGNoiseData(TString ggNoiseData);
    Bool_t InitGGNoiseSubtractor();

    Bool_t SetGainCalibrationData(TString filename, TString dataType = "f");
    void SetGainReference(Int_t row, Int_t layer);
    void SetGainReference(Double_t constant, Double_t linear, Double_t quadratic = 0.);

    Bool_t SetGainMatchingData(TString filename);
    void SetTbRange(Int_t startTb, Int_t endTb);

    Bool_t SetUAMap(TString filename);
    Bool_t SetAGETMap(TString filename);

    void SetUseSeparatedData(Bool_t value = kTRUE);

    void ProcessCobo(Int_t coboIdx);

    Bool_t SetWriteFile(TString filename, Int_t coboIdx = 0, Bool_t overwrite = kFALSE);
    void WriteData();

    STRawEvent *GetRawEvent(Long64_t eventID = -1);       ///< Returns STRawEvent object filled with the data
    Int_t GetEventID();                                   ///< Returns the current event ID
    Int_t GetNumTbs(Int_t coboIdx = 0);                   ///< Returns the number of time buckets of the data

    STMap *GetSTMap();
    STPlot *GetSTPlot();

    void GoToEnd(Int_t coboIdx = 0);
    void GenerateMetaData(Int_t runNo);
    void LoadMetaData(TString filename, Int_t coboIdx = -1);

  private:
    Int_t GetFPNChannel(Int_t chIdx);

    STMap *fMapPtr;
    STPlot *fPlotPtr;

    Int_t fNumTbs;

    GETDecoder *fDecoderPtr[12];
    Bool_t fIsData;

    STPedestal *fPedestalPtr[12];
    STGGNoiseSubtractor *fGGNoisePtr[12];
    Bool_t fIsGGNoiseGenerationMode;
    Bool_t fIsSetGGNoiseData;
    Bool_t fIsNegativePolarity;
    Double_t fFPNSigmaThreshold;

    STGainCalibration *fGainCalibrationPtr[12];
    Bool_t fIsGainCalibrationData;

    STGainMatching *fGainMatchingPtr[12];
    Bool_t fIsGainMatchingData;
    Int_t fStartTb;
    Int_t fEndTb;

    STRawEvent *fRawEventPtr;
    std::vector<STPad *> fPadArray;

    Int_t fCurrentEventID[12];
    Int_t fTargetFrameID;

    Bool_t fIsSeparatedData;

  ClassDef(STCore, 1);
};

#endif
