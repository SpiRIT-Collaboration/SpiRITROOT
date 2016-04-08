#ifndef STGENFITTRAJECTORY
#define STGENFITTRAJECTORY

// SPiRITROOT classes
#include "STHitCluster.hh"
#include "STSpacepointMeasurement.hh"
#include "STEvent.hh"
#include "STRiemannTrack.hh"
#include "STRiemannHit.hh"
#include "STTrack.hh"

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

#include "TClonesArray.h" 

class STGenfitTrajectory
{
  public:
    STGenfitTrajectory();
    ~STGenfitTrajectory();

    Bool_t SetTrack(STTrack *recoTrack, STEvent *event, STRiemannTrack *riemannTrack);
    Bool_t ProcessTrack(genfit::Track *genfitTrack);

    Bool_t SetTrack(STEvent *event, STTrack *recoTrack);
    Bool_t ExtrapolateTrack(Double_t distance, TVector3 &position);

  private:
    Int_t fTPCDetID;
    Int_t fMinIterations;
    Int_t fMaxIterations;

    TClonesArray *fHitClusterArray;
    TClonesArray *fGenfitTrackArray;

    std::vector<Int_t> *fPDGCandidateArray;

    genfit::AbsKalmanFitter *fKalmanFitter;
    genfit::MeasurementProducer<STHitCluster, genfit::STSpacepointMeasurement> *fMeasurementProducer;
    genfit::MeasurementFactory<genfit::AbsMeasurement> *fMeasurementFactory;

    genfit::SharedPlanePtr fTargetPlane;
    genfit::SharedPlanePtr fKyotoLPlane;
    genfit::SharedPlanePtr fKyotoRPlane;
    genfit::SharedPlanePtr fKatanaPlane;

    genfit::RKTrackRep *fCurrentTrackRep;
    genfit::StateOnPlane fCurrentFitState;
    Int_t fCurrentDirection;

  ClassDef(STGenfitTrajectory, 1)
};

#endif
