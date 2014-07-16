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

/**
  * This class loads RAW data file from GET electronics and
  * decodes binary data to human readable format. After that
  * according to the given mapping and pedestal data, this arranges
  * data from each channel to the SpiRIT-TPC padplane.
 **/
class STDecoderTask : public FairTask {
  public:
    //! Constructor
    STDecoderTask();
    //! Destructor
    ~STDecoderTask();

    //! Setting the number of time buckets used when taking data
    void SetNumTbs(Int_t numTbs);
    //! Setting raw data file
    void SetGraw(Char_t *filename);
    //! Setting pedestal data file. If not set, internal pedestal calculation method will be used.
    void SetPedestal(Char_t *filename);

    //! If set, decoded raw data is written in ROOT file with STRawEvent class.
    void SetPersistence(Bool_t value = kTRUE);

    //! Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    //! Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    //! Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);

  private:
    FairLogger *fLogger;          //!< FairLogger singleton

    STCore *fDecoder;             //!< STConverter pointer

    Char_t *fGrawFile;            //!< Raw data file name
    Char_t *fPedestalFile;        //!< Pedestal data file name
    Int_t fNumTbs;                //!< The number of time buckets

    Bool_t fIsPersistence;        //!< Persistence check variable

    STDigiPar *fPar;              //!< Parameter read-out class pointer
    TClonesArray *fRawEventArray; //!< STRawEvent container

  ClassDef(STDecoderTask, 1);
};

#endif
