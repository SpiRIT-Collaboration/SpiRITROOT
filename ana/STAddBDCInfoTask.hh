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

#ifndef _STADDBDCINFOTASK_H_
#define _STADDBDCINFOTASK_H_

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STData.hh"
#include "STDigiPar.hh"
#include "STVector.hh"
#include "STBeamEnergy.hh"
#include "STBDCProjection.hh"

// ROOT classes
#include "TClonesArray.h"
#include "TLorentzVector.h"
#include "TString.h"
#include "TH2.h"

// STL
#include <vector>
#include <memory>

using std::vector;

class STAddBDCInfoTask : public FairTask {
  public:
    STAddBDCInfoTask();
    /// Destructor
    ~STAddBDCInfoTask();

    void SetRunNo(int runNo) { fRegisterID = true; fRunNo = runNo; }
    void SetBeamFile(TString beamFile) { fBeamFilename = beamFile; }
    void SetZtoProject(double peakZ, double sigma, double sigmaMultiple);
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
    TClonesArray *fData;                ///< STData from the conc files

    TString fBeamFilename = "";
    TFile *fBeamFile = nullptr;
    TTree *fBeamTree = nullptr;
    Double_t fZ, fAoQ, fBeta37;
    TTree *fBDCTree;
    Double_t fBDC1x, fBDC1y, fBDC2x, fBDC2y, fBDCax, fBDCby;
    STBeamEnergy *fBeamEnergy = nullptr;

    Double_t fPeakZ = -9999;
    Double_t fSigma = 0;
    Double_t fSigmaMultiple = 0;

    int fRunNo = -1;
    TClonesArray *fEventIDArr = nullptr;
    TClonesArray *fRunIDArr = nullptr;
    int fEventID = 0;
    STBDCProjection *fBDCProjection = nullptr;
    bool fRegisterID = false;
  ClassDef(STAddBDCInfoTask, 1);
};

#endif
