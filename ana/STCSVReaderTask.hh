#ifndef _STCSVREADERTASK_H_
#define _STCSVREADERTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "STModelToLabFrameGenerator.hh"
#include "STConcReaderTask.hh"
#include "STAnaParticleDB.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TString.h"
#include "TH2.h"
#include "TTree.h"
#include "TXMLNode.h"
#include "TSystem.h"

// STL
#include <vector>

class STCSVReaderTask : public STReaderTask {
  public:
    /// Constructor
    STCSVReaderTask(TString filename);
    /// Destructor
    ~STCSVReaderTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);

    int GetNEntries();
    void SetEventID(int eventID) { fEventID = eventID; }
    std::string GetPathToData() { return std::string(gSystem -> Getenv("VMCWORKDIR")) + "/macros/data/"; };

  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    STDigiPar* fPar = nullptr;
    std::vector<STTransportParticle> fParticleList;
    std::vector<double> fdEdX;
    std::vector<int> fNClus;
    Bool_t fIsPersistence;
    
    TClonesArray *fTruthPID = nullptr;
    TClonesArray *fData = nullptr;
    int fEventID = 0;

  ClassDef(STCSVReaderTask, 1);
};

#endif
