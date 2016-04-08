//-----------------------------------------------------------
// Description:
//   Fit the track candidates found by Riemann tracking
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef STGENFITTASK
#define STGENFITTASK

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STDigiPar.hh"
#include "STHitCluster.hh"
#include "STSpacepointMeasurement.hh"
#include "STRiemannTrack.hh"

// GENFIT2 classes
#include "AbsKalmanFitter.h"
#include "KalmanFitterRefTrack.h"
#include "DAF.h"
#include "GFRaveVertexFactory.h"
#include "ConstField.h"
#include "FieldManager.h"
#include "MaterialEffects.h"
#include "TGeoMaterialInterface.h"
#include "MeasurementFactory.h"
#include "MeasurementProducer.h"
#include "EventDisplay.h"

// ROOT classes
#include "TClonesArray.h" 

class STGenfitTask : public FairTask {
  public:
    STGenfitTask();
    ~STGenfitTask();

    void SetMinIterations(Int_t value);
    void SetMaxIterations(Int_t value);
    void SetFindVertex(Bool_t value = kTRUE);
    void SetDisplay(Bool_t value = kTRUE);

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    void SetPersistence(Bool_t value = kTRUE);

    void OpenDisplay();

    Bool_t GetdEdxFromRiemann(STEvent *event, STRiemannTrack *track, Double_t &totalLength, Int_t &totalEloss);

  private:
    Bool_t fIsPersistence;  ///< Persistence check variable

    FairLogger *fLogger;

    STDigiPar *fPar;
    
    TClonesArray *fTrackArray;
    TClonesArray *fVertexArray;
    TClonesArray *fRiemannTrackArray;
    TClonesArray *fHitClusterArray;
    TClonesArray *fEventArray;

    Int_t fTPCDetID;

    genfit::MeasurementProducer<STHitCluster, genfit::STSpacepointMeasurement> *fMeasurementProducer;
    genfit::MeasurementFactory<genfit::AbsMeasurement> *fMeasurementFactory;

    Int_t fMinIterations;
    Int_t fMaxIterations;
    genfit::AbsKalmanFitter *fFitter;

    Bool_t fIsFindVertex;
    genfit::GFRaveVertexFactory *fVertexFactory;

    Bool_t fIsDisplay;
    genfit::EventDisplay *fDisplay;

    vector<Int_t> *fPDGCandidates;
  
  ClassDef(STGenfitTask, 1);
};

#endif
