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

#ifndef _STFILTEREVENTTASK_H_
#define _STFILTEREVENTTASK_H_

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
#include "TCutG.h"
#include "TFile.h"
#include "TH2.h"

// STL
#include <vector>
#include <memory>



class STFilterEventTask : public FairTask {
  public:
    STFilterEventTask();
    /// Destructor
    ~STFilterEventTask();

    void SetBeamCut(TString cutFileName, TString cutName, double acceptance=1);
    void SetBeamFor124Star(TString cutFileName); // this beam type is so special that it needs to be treated differently
    void SetVertexCut(double zMin, double zMax) { fVertexZMin = zMin; fVertexZMax = zMax; fVertexCut = true; } 
    void SetMultiplicityCut(int multMin, int multMax, double dpoca=-1) { fMultMin = multMin; fMultMax = multMax; fDPoca = dpoca; fMultCut = true; }
    void SetVertexXYCut(double xmin, double xmax, double ymin, double ymax) 
    { fXmin = xmin; fXmax = xmax; fYmin = ymin; fYmax = ymax; fPosCut = true; }
    void SetVertexBDCCut(double xmean, double xdiff, double ymean, double ydiff) 
    { fXDiff = xdiff; fXMean = xmean; fYDiff = ydiff; fYMean = ymean; fVertexBDCCut = true; }
    void SetRejectBadEvents(bool reject = true) { fRejectFastClose = reject; }
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
    TClonesArray *fEventType;
    STVectorI *fSkip;                ///< Tell other class to skip events. 1 = skip, 0 = dont' skip

    std::vector<TCutG*> fCutG;
    std::vector<double> fAcceptance;
    std::vector<TFile*> fCutFile;

    bool fVertexCut = false;
    double fVertexZMin, fVertexZMax;
    bool fMultCut = false;
    int fMultMin, fMultMax, fDPoca;
    bool fPosCut = false;
    double fXmin, fXmax, fYmin, fYmax;
    bool fVertexBDCCut = false;
    double fXDiff, fXMean, fYDiff, fYMean;
    bool fRejectFastClose = false;
    int fStatus;
    
    
  ClassDef(STFilterEventTask, 1);
};

#endif
