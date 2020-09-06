#ifndef _STDIVIDEEVENTTASK_H_
#define _STDIVIDEEVENTTASK_H_

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
#include "TString.h"
#include "TChain.h"

// STL
#include <vector>
#include <unordered_map> 

struct pair_hash
{
  template <class T1, class T2>
  std::size_t operator() (const std::pair<T1, T2> &pair) const
  {
    return std::hash<T1>()(pair.first) ^ std::hash<T2>()(pair.second);
  }
};

class STDivideEventTask : public FairTask {
  public:
    /// Constructor
    STDivideEventTask();
    /// Destructor
    ~STDivideEventTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);

    void ComplementaryTo(TString ana_filename);

  private:
    bool fIsPersistence;
    FairLogger *fLogger;                ///< FairLogger singleton
    STDigiPar* fPar = nullptr;
    
    TClonesArray *fData = nullptr;
    TClonesArray *fProb = nullptr;
    TClonesArray *fEff = nullptr;
    TClonesArray *fEventID = nullptr;
    TClonesArray *fRunID = nullptr;
    TClonesArray *fCompEventID = nullptr;

    TString fComplementaryFilename;
    TChain fComplementaryEvent;
    STVectorI *fID = nullptr;
    STVectorI *fComplementaryID = nullptr;
    STVectorI *fSkip = nullptr;
    std::unordered_map<std::pair<int, int>, int, pair_hash> fEventIDToTreeID;
    const std::vector<int> fSupportedPDG = STAnaParticleDB::GetSupportedPDG();

  ClassDef(STDivideEventTask, 1);
};

#endif
