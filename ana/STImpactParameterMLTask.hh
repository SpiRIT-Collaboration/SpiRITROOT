#ifndef _STIMPACTPARAMETERMLTASK_HH_
#define _STIMPACTPARAMETERMLTASK_HH_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "STAnaParticleDB.hh"
#include "STObsWriterTask.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TH1.h"

// others
#include "ProcessPipe.hh"

// STL
#include <vector>
#include <memory>



class STImpactParameterMLTask : public FairTask {
  public:
    STImpactParameterMLTask(const std::string& model_filename);
    /// Destructor
    ~STImpactParameterMLTask();

    void LoadTrueImpactPara(const std::string& filename);
    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();
    void SetPersistence(Bool_t value) { fIsPersistence = value; };

    // Input obs files from STObsWriter class. It is being trained with UrQMD data.
    static void TrainAlgorithm(const std::string& obs_filename, const std::string& model_filename, 
                               const std::vector<ObsType>& features={ObsType::ET, 
                                                                     ObsType::ERat, 
                                                                     ObsType::MCh, 
                                                                     ObsType::N_H_He, 
                                                                     ObsType::N_H_He_pt, 
                                                                     ObsType::N, 
                                                                     ObsType::Npt});
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
    Bool_t fUseAltObs = false;
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    STVectorF *fImpactParameterML = nullptr;
    STVectorF *fImpactParameterTruth = nullptr;
    STVectorF *fAllObs = nullptr;

    const std::string fSPIRITDIR;
    std::string fImpactParameterFilename;
    std::vector<int> fFeatureID;
    int fFeatureMaxID;
    ProcessPipe fPipe;

    
  ClassDef(STImpactParameterMLTask, 1);
};

#endif
