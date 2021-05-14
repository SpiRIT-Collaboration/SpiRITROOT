#ifndef _STPHIEFFICIENCYTASK_H_
#define _STPHIEFFICIENCYTASK_H_

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



class STPhiEfficiencyTask : public FairTask {
  public:
    STPhiEfficiencyTask();
    /// Destructor
    ~STPhiEfficiencyTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);

    void SetPersistence(Bool_t value);
    void LoadPhiEff(const std::string& eff_filename);

    static void CreatePhiEffFromData(const std::vector<std::string>& ana_filenames, const std::string& out_filename, int nClus=5, double poca=20, double bmin=0, double bmax=10);
    static void CreatePhiEffFromData(const std::string& ana_filename, const std::string& out_filename, int nClus=5, double poca=20, double bmin=0, double bmax=10);
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fData;
    TClonesArray *fCMVector;
    TClonesArray *fPhiEff;

    std::string fEffFilename;
    TFile *fEffFile = nullptr;
    std::map<int, TH2F*> fEff;
    const std::vector<int> fSupportedPDG = STAnaParticleDB::GetSupportedPDG();

    int fMinNClusters = 0;
    double fMaxDPOCA = 20;

  ClassDef(STPhiEfficiencyTask, 1);
};

#endif
