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

#ifndef STGENERATOR
#define STGENERATOR

#include "TObject.h"
#include "TString.h"

#include "STCore.hh"
#include "STParReader.hh"

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
    void SetInternalPedestal(Int_t startTb = 3, Int_t numTbs = 20);
    Bool_t SetPedestalData(TString filename, Double_t rmsFactor = 0);
    void SetFPNPedestal(Double_t fpnThreshold = 5);
    void SetPersistence(Bool_t value = kTRUE);
    void SetStoreRMS(Bool_t value = kTRUE);
    void SetSumRMSCut(Int_t value = 0);
    void SetPositivePolarity(Bool_t value = kTRUE);

    Bool_t AddData(TString filename);
    Bool_t AddData(Double_t voltage, TString filename);
    Bool_t SetData(Int_t index);

    void SelectEvents(Int_t numEvents = 0, Int_t *eventList = NULL);
    void StartProcess();

    void Print();

  private:
    void GeneratePedestalData();
    void GenerateGainCalibrationData();
    void GenerateSignalDelayData();

    enum EMode { kError, kPedestal, kGain, kSignalDelay };
    Int_t fMode;

    vector<Double_t> fVoltageArray;

    STCore *fCore;
    STParReader *fParReader;
    TString fOutputFile;

    Bool_t fIsPersistence;
    Bool_t fIsPositivePolarity;
    Bool_t fIsStoreRMS;
    Int_t fSumRMSCut;

    Int_t fNumEvents;
    Int_t *fEventList;

    Int_t fNumTbs;
    Int_t fRows;
    Int_t fLayers;
    Double_t fPadX;
    Double_t fPadZ;

  ClassDef(STGenerator, 1)
};

#endif
