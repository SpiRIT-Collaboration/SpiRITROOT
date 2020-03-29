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

#ifndef _STCONCREADERTASK_H_
#define _STCONCREADERTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TString.h"
#include "TH2.h"
#include "TTree.h"

// STL
#include <vector>

class STConcReaderTask : public FairTask {
  public:
    /// Constructor
    STConcReaderTask();
    /// Destructor
    ~STConcReaderTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);

    void SetChain(TChain* chain);
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    STDigiPar* fPar = nullptr;
    
    TTree *fChain = nullptr;
    TClonesArray *fData = nullptr;
    TClonesArray *fMCEventID = nullptr;
    TClonesArray *fEventTypeArr = nullptr;
    TClonesArray *fRunIDArr = nullptr;
    int fEventType;
    int fMCLoadedID;
    int fRunID;
    STData *fSTData = nullptr;
    Int_t  fEventID;
    Bool_t fIsPersistence;

    bool fIsTrimmedFile = false;
    bool fHasRegistered = false;

  ClassDef(STConcReaderTask, 1);
};

#endif
