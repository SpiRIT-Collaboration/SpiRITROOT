#ifndef _STRECORDIMPPARATASK_H_
#define _STRECORDIMPPARATASK_H_

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
#include "TH2.h"
#include "TH1.h"
#include "TParameter.h"

// STL
#include <vector>
#include <memory>



class STRecordImpParaTask : public FairTask {
  public:
    STRecordImpParaTask();
    /// Destructor
    ~STRecordImpParaTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    /// Write any meta data to output
    virtual void FinishTask();

    void SetPersistence(Bool_t value);
    void SetBMax(double bMax=7.5, double smear=1) { fBMax = bMax; fSmear = smear; }
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable

    double fBMax = 1;
    double fSmear = 0;
  
    STVectorF *fMultImp = nullptr;
    STVectorF *fMLImp = nullptr;
    STVectorF *fTrueImp = nullptr; ///
    STVectorI *fSkip = nullptr;

    TH1F *fMLHist = nullptr;
    TH1F *fMultHist = nullptr;
    TH1F *fTrueHist = nullptr;
    TH2F *fMLVsMult = nullptr;
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    const std::vector<int> fSupportedPDG = STAnaParticleDB::GetSupportedPDG(); /// load the array of supported particles. Your own particle information array should follows the same order
    
  ClassDef(STRecordImpParaTask, 1);
};

#endif
