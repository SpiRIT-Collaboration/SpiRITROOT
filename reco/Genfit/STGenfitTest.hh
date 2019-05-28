#ifndef STGENFITTEST
#define STGENFITTEST

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

class STGenfitTest
{
  public:
    STGenfitTest();
    STGenfitTest(Bool_t loadSamurai);
    ~STGenfitTest() {};

    void Init();
    void SetTargetPlane(TVector3 position, TVector3 normal);

    void SetMinIterations(Int_t value);
    void SetMaxIterations(Int_t value);

    genfit::Track* FitTrack(STTrack *recoTrack, STEvent *event, STRiemannTrack *riemannTrack);
    void SetTrackParameters(STTrack *recotrack, genfit::Track *genfitTrack, STEvent *event, STRiemannTrack *riemannTrack);

    Bool_t SetTrack(STEvent *event, STTrack *recoTrack);
    Bool_t SetTrack(TClonesArray *array, STTrack *recoTrack);
    Bool_t ExtrapolateTrack(Double_t distance, TVector3 &position);

  private:
    Bool_t ProcessTrack(genfit::Track *genfitTrack);
    void FindAndSetExtrapolation(STTrackCandidate *recoTrackCand);
    Bool_t CalculatedEdxFromRiemann(STEvent *event, STRiemannTrack *track, Double_t &totalLength, Double_t &totaldEdx);
    Bool_t CalculatedEdx(STTrack *recoTrack, genfit::Track *genfitTrack);

  private:
    Int_t fTPCDetID;

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
    genfit::MeasuredStateOnPlane fCurrentFitState;
    genfit::FitStatus *fCurrentFitStatus;

    STTrackCandidate *fRecoTrackCand;

    Int_t fCurrentDirection;

  ClassDef(STGenfitTest, 1)
};

#endif
