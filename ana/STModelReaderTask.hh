#ifndef _STMODELREADERTASK_H_
#define _STMODELREADERTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "STModelToLabFrameGenerator.hh"
#include "STConcReaderTask.hh"
#include "STAnaParticleDB.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TString.h"
#include "TH2.h"
#include "TTree.h"
#include "TXMLNode.h"
#include "TSystem.h"

// STL
#include <vector>

class STModelReaderTask : public STReaderTask {
  public:
    /// Constructor
    STModelReaderTask(TString filename);
    /// Destructor
    ~STModelReaderTask();

    /// Initializing the task. This will be called when Init() method invoked from FairRun.
    virtual InitStatus Init();
    /// Setting parameter containers. This will be called inbetween Init() and Run().
    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    virtual void Exec(Option_t *opt);
    void SetPersistence(Bool_t value);

    int GetNEntries();
    void SetEventID(int eventID);
    std::string GetPathToData() { return std::string(gSystem -> Getenv("VMCWORKDIR")) + "/macros/data/"; };

    void SetBeamAndTarget(int beamA, int targetA, double energyPerA);
    void RotateEvent(bool val=true) { fRotate = val; }
  private:
    FairLogger *fLogger;                ///< FairLogger singleton
    STDigiPar* fPar = nullptr;
    std::unique_ptr<STTransportReader> fReader;
    Bool_t fIsPersistence;
    
    TClonesArray *fData = nullptr;
    TClonesArray *fProb = nullptr;
    TClonesArray *fEff = nullptr;
    STVectorF *fMCRotZ = nullptr;
    std::vector<int> fSupportedPDG = STAnaParticleDB::SupportedPDG;

    TLorentzVector fFourVect;
    TVector3 fBoostVector;
    int fTargetA, fBeamA, fBeamZ;
    double fEnergyPerA;
    bool fRotate = false;

  ClassDef(STModelReaderTask, 1);
};

#endif
