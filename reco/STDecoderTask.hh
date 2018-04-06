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
    void AddData(TString filename, Int_t coboIdx = 0);
    /// Setting meta data
    void SetMetaData(TString filename, Int_t coboIdx = 0);
    /// Setting which data to be decoded
    void SetData(Int_t value);
    /// Setting the FPN RMS value cut for pedestal region taking
    void SetFPNPedestal(Double_t rms);
    /// Setting the gating grid nose data file
    void SetGGNoiseData(TString filename);
    /// Setting use gain calibration data file. If there's no file specified by user using two methods below, it'll use the one in parameter files.
    void SetUseGainCalibration(Bool_t value = kTRUE);
    /// Setting gain calibration data file. If not set, gain is not calibrated.
    void SetGainCalibrationData(TString filename);
    /// Setting gain calibration reference.
    void SetGainReference(Double_t constant, Double_t linear, Double_t quadratic = 0.);
    /// Setting gain matching data.
    void SetGainMatchingData(TString filename);
    /// Setting to decode old data file
    void SetOldData(Bool_t oldData = kTRUE);
    /// Setting to use not merged data files
    void SetUseSeparatedData(Bool_t value = kTRUE);
    /// Setting event id for STSource
    void SetEventID(Long64_t eventid = -1);
    /// Setting raw data file list
    void SetDataList(TString list);

    Long64_t GetEventID();

    /// If set, decoded raw data is written in ROOT file with STRawEvent class.
    void SetPersistence(Bool_t value = kTRUE);
    // If set, events are embedded from MC sample
    void SetEmbedding(Bool_t value = kTRUE);
    
    // MC file for embedding
    void SetEmbedFile(TString filename);
  
    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    /// Finishing the event.
    virtual void FinishEvent();

    // Embed montecarlo events
  //    STRawEvent* Embedding(TString datafile, Int_t eventId);
    
  
    /// Read event for STSource
    Int_t ReadEvent(Int_t eventID);

  private:
    FairLogger *fLogger;                ///< FairLogger singleton

    STCore *fDecoder;                   ///< STConverter pointer

    vector<TString> fDataList[12];      ///< Raw data file list
    TString fMetaData[12];              ///< Meta data files
    Int_t fDataNum;                     ///< Set which number in data list to be decoded

    Double_t fFPNPedestalRMS;           ///< RMS cut of baseline matching part selection

    TString fGGNoiseFile;               ///< Gating grid noise data file

    Bool_t fUseGainCalibration;         ///< Use gain calibration data
    TString fGainCalibrationFile;       ///< Gain calibration data file name
    Double_t fGainConstant;             ///< Gain calibration reference constant
    Double_t fGainLinear;               ///< Gain calibration reference coefficient of linear term
    Double_t fGainQuadratic;            ///< Gain calibration reference coefficient of quadratic term
    TString fGainMatchingData;          ///< Gain matching data

    Bool_t fExternalNumTbs;             ///< Flag for checking if the number of time buckets is set by the user.
    Int_t fNumTbs;                      ///< The number of time buckets

    Bool_t fIsPersistence;              ///< Persistence check variable
    Bool_t fIsEmbedding;                ///< Enabling embedding
    TString fEmbedFile;                 ///< MC file for embedding
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TChain *fChain;
    TClonesArray *fEventArray;

    TClonesArray *fEmbedTrackArray;     ///< STRecoTrack embed track exact info
    TClonesArray *fRawEventArray;       ///< STRawEvent container embeded + data
    TClonesArray *fRawEmbedEventArray;  ///< STRawEvent container of embeded event
    TClonesArray *fRawDataEventArray;   ///< STRawEvent container just data
    STRawEvent *fRawEvent;              ///< Current raw event for data + MC
    STRawEvent *fRawEventMC;            ///< Current raw event for MC run  
    STRawEvent *fRawEventData;          ///< Current raw event for just data
  
    Bool_t fOldData;                    ///< Set to decode old data
    Bool_t fIsSeparatedData;            ///< Set to use separated data files

    Long64_t fEventIDLast;              ///< Last event ID 
    Long64_t fEventID;                  ///< Event ID for STSource

  ClassDef(STDecoderTask, 1);
};

#endif
