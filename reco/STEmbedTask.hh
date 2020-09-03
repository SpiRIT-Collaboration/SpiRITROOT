//-----------------------------------------------------------
// Description:
//   Embed pulses onto the data
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//   Tommy Tsang     MSU                  (decouple this class from STDecoder class)
//-----------------------------------------------------------

#ifndef _STEMBEDASK_H_
#define _STEMBEDASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"

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

class STEmbedTask : public FairTask {
  public:
    /// Constructor
    STEmbedTask();
    /// Destructor
    ~STEmbedTask();

    /// Setting the number of time buckets used when taking data
    // MC file for embedding
    void SetEmbedFile(TString filename);
  
    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);


    void SetEventID(Long64_t eventid);
  private:
    FairLogger *fLogger;                ///< FairLogger singleton

    Bool_t fIsPersistence;              ///< Persistence check variable
    TString fEmbedFile;                 ///< MC file for embedding
    Int_t  fEventID;                    ///< Data event id
  
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

  FairMCEventHeader *fMCEventHeader;
  
  ClassDef(STEmbedTask, 1);
};

#endif
