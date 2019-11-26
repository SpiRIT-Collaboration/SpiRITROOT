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

#ifndef _STEFFICIENCYTASK_H_
#define _STEFFICIENCYTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "EfficiencyFactory.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TH2.h"

// STL
#include <vector>
#include <memory>

using std::vector;

class STEfficiencyTask : public FairTask {
  public:
    enum Particles { Pim, Pip, Proton, Triton, He3, END};//, Proton, Triton, He3, END };
    /// Constructor
    STEfficiencyTask();
    /// Destructor
    ~STEfficiencyTask();

    void EfficiencySettings(int t_num_clusters,
                            double t_dist_2_vert,
                            int t_min_mult,
                            int t_max_mult,
                            const std::vector<std::pair<double, double>>& t_phi_cut={},
                            const std::vector<std::pair<double, double>>& t_phi_range={},
                            const std::string& t_efficiency_db="DataBase/EfficiencyDB.root",
                            const std::string& t_cut_db="",
                            double t_cluster_ratio=0);

    void SetMomBins(Particles part, double t_min, double t_max, int t_bins);
    void SetThetaBins(Particles part, double t_min, double t_max, int t_bins); 

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);

  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
  
    STDigiPar *fPar;                    ///< Parameter read-out class pointer
    TClonesArray *fPDG;                 ///<
    TClonesArray *fData;                ///< STData from the conc files
    TClonesArray *fEff;                 ///< Efficiency of each type of particle

    const std::vector<std::string> fParticleNameEff = {"pi-", "pi+", "proton", "triton", "he3"};
    const std::vector<int> fSupportedPDG = {-211, 211, 2212, 1000010030, 1000020030};
    std::vector<std::unique_ptr<EfficiencyFactory>> fEfficiencyFactory; ///<
    std::vector<TEfficiency> fEfficiency; ///<
  
  ClassDef(STEfficiencyTask, 1);
};

#endif
