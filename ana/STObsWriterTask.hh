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

#ifndef _STOBSWRITERTASK_H_
#define _STOBSWRITERTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "STAnaParticleDB.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TH1.h"

// STL
#include <vector>
#include <memory>



class STObsWriterTask : public FairTask {
  public:
    STObsWriterTask(const std::string& output_name);
    /// Destructor
    ~STObsWriterTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value) { fIsPersistence = value; };
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    std::ofstream fOutput;
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fProb;
    TClonesArray *fCMVector;
    TClonesArray *fData;
    TClonesArray *fFragRapidity;
    STVectorF *fBeamRapidity;
    STVectorF *fERAT;
    STVectorF *fET;
    STVectorF *fAllObs;
    

    const std::vector<int> fSupportedPDG = STAnaParticleDB::GetSupportedPDG();
    
  ClassDef(STObsWriterTask, 1);
};

#endif
