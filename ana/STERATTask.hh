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

#ifndef _STERATTASK_H_
#define _STERATTASK_H_

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



class STERATTask : public FairTask {
  public:
    STERATTask();
    /// Destructor
    ~STERATTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);
    void SetImpactParameterTable(const std::string& table_filename);

    static void CreateImpactParameterTable(const std::string& ana_filename, const std::string& output_filename);
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fProb;
    TClonesArray *fCMVector;
    TClonesArray *fData;
    STVectorF *fERAT;
    STVectorF *fbERat; // impact parameter from ERAT
    STVectorF *fbMult; // impact parameter from multiplicity

    const std::vector<int> fSupportedPDG = STAnaParticleDB::SupportedPDG;

    std::string fImpactParameterFilename;
    TFile *fImpactParameterFile = nullptr;
    TH1F *fMultHist = nullptr;
    TH1F *fERatHist = nullptr;
    
  ClassDef(STERATTask, 1);
};

#endif
