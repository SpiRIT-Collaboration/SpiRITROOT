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
    /// Setting pedestal data file. If not set, internal pedestal calculation method will be used.
    void SetPedestalData(TString filename);
    /// Setting gain calibration data file. If not set, gain is not calibrated.
    void SetGainCalibrationData(TString filename);
    /// Setting gain calibration base.
    void SetGainBase(Double_t constant, Double_t slope);
    /// Setting signal delay data file. If not set, signal is not delayed.
    void SetSignalDelayData(TString filename);

    /// If set, decoded raw data is written in ROOT file with STRawEvent class.
    void SetPersistence(Bool_t value = kTRUE);

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);

  private:
    FairLogger *fLogger;          /// FairLogger singleton

    STCore *fDecoder;             /// STConverter pointer

    vector<TString> fDataList;    /// Raw data file list
    Int_t fDataNum;               /// Set which number in data list to be decoded
    TString fPedestalFile;        /// Pedestal data file name

    TString fGainCalibrationFile; /// Gain calibration data file name
    Double_t fGainConstant;       /// Gain calibration base constant
    Double_t fGainSlope;          /// Gain calibration base slope

    TString fSignalDelayFile;     /// Signal Delay data file name
    Int_t fNumTbs;                /// The number of time buckets

    Bool_t fIsPersistence;        /// Persistence check variable

    STDigiPar *fPar;              /// Parameter read-out class pointer
    TClonesArray *fRawEventArray; /// STRawEvent container

  ClassDef(STDecoderTask, 1);
};

#endif
