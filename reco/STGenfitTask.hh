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

// GENFIT2 classes
#include "AbsKalmanFitter.h"
#include "KalmanFitterRefTrack.h"
#include "GFRaveVertexFactory.h"
#include "ConstField.h"
#include "FieldManager.h"
#include "MaterialEffects.h"
#include "TGeoMaterialInterface.h"
#include "MeasurementFactory.h"
#include "MeasurementProducer.h"

// ROOT classes
#include "TClonesArray.h" 

class STGenfitTask : public FairTask {
  public:
    STGenfitTask();
    ~STGenfitTask();

    void SetMaxIterations(Int_t value);
    void SetFindVertex(Bool_t value = kTRUE);

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
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

    Int_t fMaxIterations;
    genfit::AbsKalmanFitter *fFitter;

    Bool_t fIsFindVertex;
    genfit::GFRaveVertexFactory *fVertexFactory;

  ClassDef(STGenfitTask, 1);
};

#endif
