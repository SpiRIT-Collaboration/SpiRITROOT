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
    STModelReaderTask(TString filename, bool enable_neutrons=false);
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
    TClonesArray *fEventID = nullptr;
    TClonesArray *fRunID = nullptr;
    STVectorF *fMCRotZ = nullptr;
    STVectorVec3 *fHvyResVec = nullptr;
    STVectorI *fHvyResPDG = nullptr;
    std::vector<int> fSupportedPDG;

    TLorentzVector fFourVect;
    TVector3 fBoostVector;
    int fTargetA, fBeamA, fBeamZ;
    double fEnergyPerA;
    bool fEnableNeutrons = false;
    bool fRotate = false;
    std::vector<int> fTreeIDList;

  ClassDef(STModelReaderTask, 1);
};

#endif
