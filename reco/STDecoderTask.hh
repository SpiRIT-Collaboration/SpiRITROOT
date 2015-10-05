//-----------------------------------------------------------
// Description:
//   Converting GRAW file to tree structure to make it easy
//   to access the data
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef _STDECODERTASK_H_
#define _STDECODERTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STCore.hh"
#include "STMap.hh"
#include "STPedestal.hh"
#include "STRawEvent.hh"

#include "STDigiPar.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TString.h"

// STL
#include <vector>

using std::vector;

/**
  * This class loads RAW data file from GET electronics and
  * decodes binary data to human readable format. After that
  * according to the given mapping and pedestal data, this arranges
  * data from each channel to the SpiRIT-TPC padplane.
 **/
class STDecoderTask : public FairTask {
  public:
    /// Constructor
    STDecoderTask();
    /// Destructor
    ~STDecoderTask();

    /// Setting the number of time buckets used when taking data
    void SetNumTbs(Int_t numTbs);
    /// Adding raw data file to the list
    void AddData(TString filename);
    /// Setting which data to be decoded
    void SetData(Int_t value);
    /// Setting values for internal pedestal calculation.
    void SetInternalPedestal(Int_t startTb = 3, Int_t averageTbs = 20);
    /// Setting pedestal data file for external pedestal data.
    void SetPedestalData(TString filename, Double_t rmsFactor = 0);
    /// Setting to use FPN channels as pedestal with RMS cut in parameters file.
    void SetFPNPedestal();
    /// Setting to use FPN channels as pedestal
    void SetFPNPedestal(Double_t rms);
    /// Setting gain calibration data file. If not set, gain is not calibrated.
    void SetGainCalibrationData(TString filename);
    /// Setting gain calibration reference.
    void SetGainReference(Double_t constant, Double_t linear, Double_t quadratic = 0.);
    /// Setting to decode old data file
    void SetOldData(Bool_t oldData = kTRUE);
    /// Setting event id for STSource
    void SetEventID(Long64_t eventid = -1);

    Long64_t GetEventID();

    /// If set, decoded raw data is written in ROOT file with STRawEvent class.
    void SetPersistence(Bool_t value = kTRUE);

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    /// Finishing the event.
    virtual void FinishEvent();

  private:
    FairLogger *fLogger;                ///< FairLogger singleton

    STCore *fDecoder;                   ///< STConverter pointer

    vector<TString> fDataList;          ///< Raw data file list
    Int_t fDataNum;                     ///< Set which number in data list to be decoded

    Bool_t fUseInternalPedestal;        ///< Flag for using internal pedestal calculation
    Int_t fPedestalStartTb;             ///< Starting time bucket number for internal pedestal calculation
    Int_t fAverageTbs;                  ///< The number of time buckets for internal pedestal calculation
    TString fPedestalFile;              ///< Pedestal data file name
    Double_t fPedestalRMSFactor;        ///< Pedestal RMS factor that will be multiplied to external pedestal RMS value
    Bool_t fUseFPNPedestal;             ///< Flas for using FPN channel as pedestal
    Double_t fFPNPedestalRMS;           ///< RMS cut of baseline matching part selection

    TString fGainCalibrationFile;       ///< Gain calibration data file name
    Double_t fGainConstant;             ///< Gain calibration reference constant
    Double_t fGainLinear;               ///< Gain calibration reference coefficient of linear term
    Double_t fGainQuadratic;            ///< Gain calibration reference coefficient of quadratic term

    Bool_t fExternalNumTbs;             ///< Flag for checking if the number of time buckets is set by the user.
    Int_t fNumTbs;                      ///< The number of time buckets

    Bool_t fIsPersistence;              ///< Persistence check variable

    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fRawEventArray;       ///< STRawEvent container
    STRawEvent *fRawEvent;              ///< Current raw event for run

    Bool_t fOldData;                    ///< Set to decode old data

    Long64_t fEventID;                  ///< Event ID for STSource

  ClassDef(STDecoderTask, 1);
};

#endif
