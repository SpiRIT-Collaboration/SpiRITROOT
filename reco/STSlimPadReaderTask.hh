//-----------------------------------------------------------
// Description:
//   Reading ROOT file containing STSlimPad, which is
//   the software zero-suppressed data
//
// Author List:
//   Genie Jhang     Unemployed     (original author)
//-----------------------------------------------------------

#ifndef _STSLIMPADREADERTASK_H_
#define _STSLIMPADREADERTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STRawEvent.hh"
#include "STGainCalibration.hh"
#include "STSlimPad.hh"

#include "STDigiPar.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TString.h"

// STL
#include <vector>

using std::vector;

/**
  * This class reads ROOT file containing STSlimPad
  * to create STRawEvent which is the same structure from
  * STDecoderTask.
 **/
class STSlimPadReaderTask : public FairTask {
  public:
    /// Constructor
    STSlimPadReaderTask();
    /// Destructor
    ~STSlimPadReaderTask();

    /// Setting the number of time buckets used when taking data
    void SetNumTbs(Int_t numTbs);
    /// Adding ROOT file to the list
    void AddData(TString filename);
    /// Setting use gain calibration data file. If there's no file specified by user using two methods below, it'll use the one in parameter files.
    void SetUseGainCalibration(Bool_t value = kTRUE);
    /// Setting gain calibration data file
    void SetGainCalibrationData(TString filename);
    /// Setting gain calibration reference
    void SetGainReference(Double_t constant, Double_t linear, Double_t quadratic = 0.);
    /// Setting to decode old data file
    void SetEventID(Long64_t eventid = -1);

    Long64_t GetEventID();

    /// If set, decoded raw data is written in ROOT file with STRawEvent class.
    void SetPersistence(Bool_t value = kTRUE);

    STRawEvent *GetRawEvent(Long64_t eventid = -1);

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    /// Finishing the event.
    virtual void FinishEvent();

    /// Read event for STSource
    Int_t ReadEvent(Int_t eventID);

  private:
    FairLogger *fLogger;                ///< FairLogger singleton

    TChain *fChain;                     ///< TTree handler
    std::vector<STSlimPad> *fPads;      ///< Pads vector

    vector<TString> fDataList;          ///< Raw data file list
    Int_t fNumEvents;                   ///< The number of events in the file

    STGainCalibration *fGainCalib;      ///< Gain calibrator
    Bool_t fUseGainCalibration;         ///< Use gain calibration data
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
    STRawEvent *fRawEventLoop;          ///< Raw event pointer for loop

    Long64_t fEventIDLast;              ///< Last event ID 
    Long64_t fEventID;                  ///< Event ID for STSource

  ClassDef(STSlimPadReaderTask, 1);
};

#endif
