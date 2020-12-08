
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
   enum TCArrType{ DATA, PROB, SD, EFF, CMVECTOR, LABRAPIDITY, FRAGRAPIDITY, FRAGVELOCITY, EFFERR, PHIEFF, V1RPANGLE, V2RPANGLE, ARREND };
   enum VecType { BEAMRAPIDITY, BEAMMOM, VECEND };
   void CheckEmptyElements(int n_particle_type);
   void UpdateData(int part_id);

   // access data
   const STData& Data() const                     { return *static_cast<STData*>(fTCArrList[DATA] -> At(0)); };
   float Eff(int n) const                         { return static_cast<STVectorF*>(fTCArrList[EFF] -> At(fPartID)) -> fElements[n]; };
   float EffErr(int n) const                      { return static_cast<STVectorF*>(fTCArrList[EFFERR] -> At(fPartID)) -> fElements[n]; };
   float Prob(int n) const                        { return static_cast<STVectorF*>(fTCArrList[PROB] -> At(fPartID)) -> fElements[n]; };
   float StdDev(int n) const                        { return static_cast<STVectorF*>(fTCArrList[SD] -> At(fPartID)) -> fElements[n]; };

   const TVector3& CMVector(int n) const          { return static_cast<STVectorVec3*>(fTCArrList[CMVECTOR] -> At(fPartID)) -> fElements[n]; };
   float LabRapidity(int n) const                 { return static_cast<STVectorF*>(fTCArrList[LABRAPIDITY] -> At(fPartID)) -> fElements[n]; };
   const TVector3& FragVelocity(int n) const      { return static_cast<STVectorVec3*>(fTCArrList[FRAGVELOCITY] -> At(fPartID)) -> fElements[n]; };
   float FragRapidity(int n) const                { return static_cast<STVectorF*>(fTCArrList[FRAGRAPIDITY] -> At(fPartID)) -> fElements[n]; };
   float PhiEff(int n) const                      { return static_cast<STVectorF*>(fTCArrList[PHIEFF] -> At(fPartID)) -> fElements[n]; };
   float V1RPAngle(int n) const                   { return static_cast<STVectorF*>(fTCArrList[V1RPANGLE] -> At(fPartID)) -> fElements[n]; };
   float V2RPAngle(int n) const                   { return static_cast<STVectorF*>(fTCArrList[V2RPANGLE] -> At(fPartID)) -> fElements[n]; };



   float Weight(int n) const                      { return fWeight[n]; };
   float PtxRap(int n) const                      { return fPtxRap[n]; };
   const std::vector<float>& BeamRapidity() const { return fVecList[BEAMRAPIDITY] -> fElements; };
   const std::vector<float>& BeamMom() const      { return fVecList[BEAMMOM] -> fElements; };

   std::vector<TClonesArray*> fTCArrList;
   std::vector<STVectorF*> fVecList;
   int fPartID; // indicates which element of the tclones array needs to be loaded
   std::vector<float> fWeight; // prob/ 
   std::vector<float> fPtxRap; // x-rapidity distribution. Extended from transverse rapidity assuming uniform phi dist
   std::map<TCArrType, double> fDefaultValues{{EFF, 1}, {EFFERR, 0}, {PHIEFF, 1}, {V1RPANGLE, 0}, {V2RPANGLE, 0}, {SD, 0}};
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
        auto hist = this -> RegisterHistogram<T>(true, (fParticleName.at(pdg) + suffix).c_str(), args...);
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
    void RegisterMCNPPlots();
    void SetNNFrame(bool val=true) { fNNFrame = val; };
    void DiscardData(const std::vector<int>& type) { fTypeToDiscard = type; };

    static void CreateMCEventsFromHist(const std::string& forwardFile, const std::string& backwardFile,
                                       const std::string& outputFile, int nevent, double energyPerN,
                                       int multMin, int multMax, const std::string& simPara);

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    virtual void FinishTask();
    void SetPersistence(Bool_t value);

    // parameters for graph drawing
    // for MC CIN CIP
    Double_t fPtThresholdForMC = 150;
    // this one is for drawing flow
    Double_t fMidRapThresholdForVs = 0.5;
    Double_t fPtThresholdForVs = 0.;
    Double_t fProbThresholdForVs = 0.95;
    Double_t fPhiEffThresholdForVs = 0.2;
    // this one is for rapidity plots
    Double_t fProbThresholdForRap = 0.2;
    Double_t fSDThresholdForRap = 1000;
    Double_t fEffThresholdForRap = 0.05;
    Double_t fPtThresholdForRap = 0;
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


  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    Bool_t fIsPersistence;              ///< Persistence check variable
    int fEntries;
    bool fPlotRapidity = false;
    bool fPlotPID = false;
    bool fPlotVs = false;
    bool fPlotPion = false;
    bool fNNFrame = false;
  
    STDigiPar   *fPar  = nullptr;                 ///< Parameter read-out class pointer
    STVectorI   *fSkip = nullptr;
    DataPackage fDataPackage;

    std::map<int, double> fMinMomForCMInLab;
    static const std::map<int, std::string> fParticleName;

    int _ToPitchId(const TVector3& vec);
    int _ToYawId(const TVector3& vec);
    const std::vector<int> fSupportedPDG = STAnaParticleDB::GetSupportedPDG();
    std::map<std::string, TH1*> f1DHists;
    std::map<std::string, bool> fNormalize;
    std::vector<std::vector<std::function<void(const DataPackage&)>>> fFillRules;
    std::vector<std::function<void()>> fFinishTaskRule;
    std::vector<int> fTypeToDiscard;

  ClassDef(STSimpleGraphsTask, 1);
};

#endif
