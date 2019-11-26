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

#ifndef _STMATCHIMQMDTASK_H_
#define _STMATCHIMQMDTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STTransportModelEventGenerator.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TString.h"
#include "TH2.h"
#include "TTree.h"

// STL
#include <vector>

using std::vector;

class STMatchImQMDTask : public FairTask {
  public:
    /// Constructor
    STMatchImQMDTask();
    /// Destructor
    ~STMatchImQMDTask();

  
    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);
    void SetImQMDFile(const std::string& t_qmdFile, const std::string& t_qmdName="ImQMD");
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    STDigiPar* fPar = nullptr;
    
    TChain *fChain = nullptr;
    TClonesArray *fEventID = nullptr;
    TClonesArray *fData = nullptr;
    TClonesArray *fQMDPart = nullptr;
    TClonesArray *fQMDMom = nullptr;
    TClonesArray *fQMDPDG = nullptr;
    TClonesArray *fQMDDReco = nullptr;
    Bool_t fIsPersistence;

  ClassDef(STMatchImQMDTask, 1);
};

#endif
