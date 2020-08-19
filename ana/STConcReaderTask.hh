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
#include "TXMLNode.h"

// STL
#include <vector>

class STReaderTask : public FairTask {
  public:
    virtual int GetNEntries() = 0;
    virtual void SetEventID(int eventID) = 0;
    virtual std::string GetPathToData() = 0;
  ClassDef(STReaderTask, 1);
};



class STConcReaderTask : public STReaderTask {
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
    void LoadFromXMLNode(TXMLNode *node); // will return path to data
    std::string GetPathToData() { return fPathToData; };
    int GetNEntries();
    void SetEventID(int eventID);
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
    STVectorF *fMCRotZ = nullptr;
    Int_t  fEventID;
    Bool_t fIsPersistence;

    bool fIsTrimmedFile = false;
    bool fHasRegistered = false;
    std::string fPathToData = "";

  ClassDef(STConcReaderTask, 1);
};

#endif
