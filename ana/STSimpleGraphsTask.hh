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

#ifndef _STSIMPLEGRAPHTASK_H_
#define _STSIMPLEGRAPHTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "EfficiencyFactory.hh"
#include "STAnaParticleDB.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TH2.h"
#include "TDatabasePDG.h"

// STL
#include <vector>
#include <memory>

struct DataPackage
{
  DataPackage(std::vector<float>& t_labRapidity,
              std::vector<float>& t_eff,
              std::vector<float>& t_prob,
              std::vector<float>& t_fragRapidity,
              std::vector<TVector3>& t_cmVector,
              std::vector<TVector3>& t_fragVelocity,
              std::vector<float>& t_beamRapidity);
  std::vector<float>& labRapidity, eff, prob, fragRapidity;
  std::vector<TVector3>& cmVector, fragVelocity;
  std::vector<float>& beamRapidity;

  std::vector<float> weight, ptRap;
 
  ClassDef(DataPackage, 1);
};


class STSimpleGraphsTask : public FairTask {
  public:
    /// Constructor
    STSimpleGraphsTask();
    /// Destructor
    ~STSimpleGraphsTask();

    template<class T, class ...Args>
    T* RegisterHistogram(Args... args)
    { auto hist = new T(args...); f1DHists[std::string(hist -> GetName())] = static_cast<TH1*>(hist); return hist; }
    
    // the name of the histogram will be prefixed by the particle name
    template<class T, class ...Args>
    void RegisterRuleForEachParticle(std::function<void(const DataPackage&, const STData&, TH1*, int)> rule, const std::string& suffix, Args... args)
    {
      for(auto pdg : fSupportedPDG)
      {
        auto hist = this -> RegisterHistogram<T>((fParticleName[pdg] + suffix).c_str(), args...);
        this -> RegisterRuleWithParticle(pdg, [pdg, rule, hist](const DataPackage& package, const STData& data){ rule(package, data, hist, pdg); });
      } 
    };

    void RegisterRuleWithParticle(int pdg, std::function<void(const DataPackage&, const STData&)> rule);
    void RegisterRapidityPlots();
    void RegisterPIDPlots();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();
    void SetPersistence(Bool_t value);
    void IgnoreMinMom(bool value) { fIgnoreMinMom = value; };
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
    int fEntries;
    bool fPlotRapidity = false;
    bool fPlotPID = false;
    bool fIgnoreMinMom = false;
  
    STDigiPar *fPar      = nullptr;                 ///< Parameter read-out class pointer
    TClonesArray *fData  = nullptr;                 ///< STData from the conc files
    TClonesArray *fEff   = nullptr;                 ///< Efficiency of each type of particle
    TClonesArray *fProb  = nullptr;                 ///< Particle PID from any of the PID Task
    TClonesArray *fLabRapidity = nullptr;
    TClonesArray *fFragRapidity = nullptr;
    TClonesArray *fFragVelocity = nullptr;
    TClonesArray *fCMVector = nullptr;
    STVectorI    *fSkip = nullptr;
    STVectorF    *fBeamRapidity = nullptr;

    std::map<int, double> fMinMomForCMInLab;
    std::map<int, std::string> fParticleName{{2212, "p"}, 
                                             {1000010020, "d"}, 
                                             {1000010030, "t"}, 
                                             {1000020030, "He3"}, 
                                             {1000020040, "He4"}, 
                                             {1000020060, "He6"}};

    // For drawing PIDs
    int fNYaw = 6;
    int fNPitches = 5;

    Int_t fMomBins = 1000;
    Int_t fMinMom = -700;
    Int_t fMaxMom = 4500;

    Int_t fdEdXBins = 1000;
    Int_t fMindEdX = 0;
    Int_t fMaxdEdX = 1500;

    Int_t fMinNClus = 15;
    Double_t fMaxDPOCA = 15;

    int _ToPitchId(const TVector3& vec);
    int _ToYawId(const TVector3& vec);
    const std::vector<int> fSupportedPDG = STAnaParticleDB::SupportedPDG;
    std::map<std::string, TH1*> f1DHists;
    std::vector<std::vector<std::function<void(const DataPackage&, const STData&)>>> fFillRules;

  ClassDef(STSimpleGraphsTask, 1);
};

#endif
