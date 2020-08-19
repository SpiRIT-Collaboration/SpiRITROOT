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
#include "TH1.h"
#include "TH2.h"
#include "TDatabasePDG.h"

// STL
#include <vector>
#include <memory>

struct DataPackage
{
   DataPackage();
   enum TCArrType{ DATA, PROB, EFF, CMVECTOR, LABRAPIDITY, FRAGRAPIDITY, FRAGVELOCITY, EFFERR, PHIEFF, ARREND };
   enum VecType { BEAMRAPIDITY, BEAMMOM, VECEND };
   void CheckEmptyElements(int n_particle_type);
   void UpdateData(int part_id);

   // access data
   const STData& Data() const                     { return *static_cast<STData*>(fTCArrList[DATA] -> At(0)); };
   float Eff(int n) const                         { return static_cast<STVectorF*>(fTCArrList[EFF] -> At(fPartID)) -> fElements[n]; };
   float EffErr(int n) const                      { return static_cast<STVectorF*>(fTCArrList[EFFERR] -> At(fPartID)) -> fElements[n]; };
   float Prob(int n) const                        { return static_cast<STVectorF*>(fTCArrList[PROB] -> At(fPartID)) -> fElements[n]; };
   const TVector3& CMVector(int n) const          { return static_cast<STVectorVec3*>(fTCArrList[CMVECTOR] -> At(fPartID)) -> fElements[n]; };
   float LabRapidity(int n) const                 { return static_cast<STVectorF*>(fTCArrList[LABRAPIDITY] -> At(fPartID)) -> fElements[n]; };
   const TVector3& FragVelocity(int n) const      { return static_cast<STVectorVec3*>(fTCArrList[FRAGVELOCITY] -> At(fPartID)) -> fElements[n]; };
   float FragRapidity(int n) const                { return static_cast<STVectorF*>(fTCArrList[FRAGRAPIDITY] -> At(fPartID)) -> fElements[n]; };
   float PhiEff(int n) const                      { return static_cast<STVectorF*>(fTCArrList[PHIEFF] -> At(fPartID)) -> fElements[n]; };

   float Weight(int n) const                      { return fWeight[n]; };
   float PtxRap(int n) const                      { return fPtxRap[n]; };
   const std::vector<float>& BeamRapidity() const { return fVecList[BEAMRAPIDITY] -> fElements; };
   const std::vector<float>& BeamMom() const      { return fVecList[BEAMMOM] -> fElements; };

   std::vector<TClonesArray*> fTCArrList;
   std::vector<STVectorF*> fVecList;
   int fPartID; // indicates which element of the tclones array needs to be loaded
   std::vector<float> fWeight; // prob/ 
   std::vector<float> fPtxRap; // x-rapidity distribution. Extended from transverse rapidity assuming uniform phi dist

};

class STSimpleGraphsTask : public FairTask {
  public:
    /// Constructor
    STSimpleGraphsTask();
    /// Destructor
    ~STSimpleGraphsTask();

    template<class T, class ...Args>
    T* RegisterHistogram(bool normalize, Args... args)
    { 
      auto hist = new T(args...); 
      f1DHists[std::string(hist -> GetName())] = static_cast<TH1*>(hist); 
      fNormalize[std::string(hist -> GetName())] = normalize;
      return hist; 
    }
    
    // the name of the histogram will be prefixed by the particle name
    template<class T, class ...Args>
    void RegisterRuleForEachParticle(std::function<void(const DataPackage&, TH1*, int)> rule, const std::string& suffix, Args... args)
    {
      for(auto pdg : fSupportedPDG)
      {
        auto hist = this -> RegisterHistogram<T>(true, (fParticleName[pdg] + suffix).c_str(), args...);
        this -> RegisterRuleWithParticle(pdg, [pdg, rule, hist](const DataPackage& package){ rule(package, hist, pdg); });
      } 
    };

    void RegisterRuleWithParticle(int pdg, std::function<void(const DataPackage&)> rule);
    void RegisterFinishTaskRule(std::function<void()> rule);
    void RegisterVPlots();
    void RegisterRapidityPlots();
    void RegisterPlotsForMC();
    void RegisterPIDPlots();
    void RegisterPionPlots();
    void RemoveParticleMin();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();
    void SetPersistence(Bool_t value);

  
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
    int fEntries;
    bool fPlotRapidity = false;
    bool fPlotPID = false;
    bool fPlotVs = false;
    bool fPlotPion = false;
  
    STDigiPar   *fPar  = nullptr;                 ///< Parameter read-out class pointer
    STVectorI   *fSkip = nullptr;
    DataPackage fDataPackage;

    std::map<int, double> fMinMomForCMInLab;
    std::map<int, std::string> fParticleName{{2212, "p"}, 
                                             {1000010020, "d"}, 
                                             {1000010030, "t"}, 
                                             {1000020030, "He3"}, 
                                             {1000020040, "He4"}, 
                                             {1000020060, "He6"},
                                             {211, "pip"},
                                             {-211, "pim"}};

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
    std::map<std::string, bool> fNormalize;
    std::vector<std::vector<std::function<void(const DataPackage&)>>> fFillRules;
    std::vector<std::function<void()>> fFinishTaskRule;

  ClassDef(STSimpleGraphsTask, 1);
};

#endif
