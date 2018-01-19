#ifndef STGENFITTEST2_HH
#define STGENFITTEST2_HH

// SPiRITROOT classes
#include "STHitCluster.hh"
#include "STSpacepointMeasurement.hh"
#include "STEvent.hh"
#include "STHelixTrack.hh"
#include "STTrack.hh"
#include "STTrackCandidate.hh"
#include "STdEdxPoint.hh"
#include "STRecoTrack.hh"

// GENFIT2 classes
#include "AbsKalmanFitter.h"
#include "KalmanFitterRefTrack.h"
#include "DAF.h"
#include "GFRaveVertexFactory.h"
#include "ConstField.h"
#include "STGFBField.hh"
#include "FieldManager.h"
#include "MaterialEffects.h"
#include "TGeoMaterialInterface.h"
#include "MeasurementFactory.h"
#include "MeasurementProducer.h"
#include "EventDisplay.h"

#include "TClonesArray.h" 

class STGenfitTest2
{
  public:
    STGenfitTest2();
    STGenfitTest2(bool loadSamurai);
    ~STGenfitTest2();

    void SetMinIterations(Int_t value);
    void SetMaxIterations(Int_t value);

    void Init();
    genfit::Track* FitTrack(STHelixTrack *helixTrack, Int_t pdg = 2212);
    genfit::Track* FitTrackWithVertex(STHelixTrack *helixTrack, STHitCluster *vertex, Int_t pdg = 2212);

    void GetTrackParameters(genfit::Track *gfTrack, TVector3 &mom, TVector3 &momentumTargetPlane, TVector3 &posTargetPlane);
    void GetPosOnPlanes(genfit::Track *gfTrack, TVector3 &kyotoL, TVector3 &kyotoR, TVector3 &katana);
    void GetMomentumWithVertex(genfit::Track *gfTrack, TVector3 vertex, TVector3 &momVertex, TVector3 &pocaVertex, Double_t &charge);

    bool GetdEdxPointsByLength(genfit::Track *gfTrack, STHelixTrack *helixTrack, vector<STdEdxPoint> *dEdxPointArray);
    bool GetdEdxPointsByLayerRow(genfit::Track *gfTrack, STHelixTrack *helixTrack, vector<STdEdxPoint> *dEdxPointArray);

    /// Set genfit-track to use ExtrapolateTrack() method.
    genfit::Track* SetTrack(STRecoTrack *recoTrack, TClonesArray *clusterArray);

    /// Track given from SetTrack(), is extrapolated by given 'distance(mm)'.
    /// If extrapolation success, return true and set 'position(mm)'. If it fails, return false.
    bool ExtrapolateTrack(Double_t distance, TVector3 &position, Int_t direction = 1);

    /// Track given from SetTrack(), is extrapolated to 'to(mm)'.
    /// If extrapolation success, return true and set 'position(mm)'. If it fails, return false.
    bool ExtrapolateTo(TVector3 to, TVector3 &position);

  private:
    Int_t DetermineCharge(genfit::Track *gfTrack);

  private:
    Int_t fTPCDetID;

    TClonesArray *fHitClusterArray;
    TClonesArray *fGenfitTrackArray;

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

  ClassDef(STGenfitTest2, 1)
};

#endif
