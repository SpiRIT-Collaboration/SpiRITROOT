#include "STGenfitTestE.hh"

// SPiRITROOT classes
#include "STGenfitTask.hh"
#include "STTrackCandidate.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"

// GENFIT2 classes
#include "Track.h"
#include "TrackCand.h"
#include "RKTrackRep.h"
#include "Exception.h"

// STL
#include <iostream>
#include <vector>

// ROOT classes
#include "TMatrixDSym.h"
#include "TMatrixD.h"
#include "TMath.h"
#include "TGeoManager.h"
#include "Math/DistFunc.h"
#include "STDatabasePDG.hh"

using namespace std;

ClassImp(STGenfitTestE)

STGenfitTestE::STGenfitTestE()
:STGenfitTestE(kTRUE)
{
}

STGenfitTestE::STGenfitTestE(Bool_t loadSamurai)
{
  fTPCDetID = 0;

  //fKalmanFitter = new genfit::DAF();
  fKalmanFitter = new genfit::KalmanFitterRefTrack();
  fKalmanFitter -> setMinIterations(5);
  fKalmanFitter -> setMaxIterations(20);

  fHitClusterArray = new TClonesArray("STHitCluster");
  fGenfitTrackArray = new TClonesArray("genfit::Track");

  fMeasurementProducer = new genfit::MeasurementProducer<STHitCluster, genfit::STSpacepointMeasurement>(fHitClusterArray);
  fMeasurementFactory = new genfit::MeasurementFactory<genfit::AbsMeasurement>();
  fMeasurementFactory -> addProducer(fTPCDetID, fMeasurementProducer);

  if (loadSamurai)
    genfit::FieldManager::getInstance() -> init(STGFBField::GetInstance("samurai_field_map","A"));
  else
    genfit::FieldManager::getInstance() -> init(new genfit::ConstField(0., 5., 0.));

  genfit::MaterialEffects *materialEffects = genfit::MaterialEffects::getInstance();
  materialEffects -> init(new genfit::TGeoMaterialInterface());

  TVector3 posTarget(0, -21.33, -0.89);
  TVector3 normalTarget(0, 0, 1);
  fTargetPlane = genfit::SharedPlanePtr(new genfit::DetPlane(posTarget, normalTarget));

  TVector3 posKyotoL(75.8, -21.33, 84.5);
  TVector3 normalKyotoL(-1, 0, 0);
  fKyotoLPlane = genfit::SharedPlanePtr(new genfit::DetPlane(posKyotoL, normalKyotoL));

  TVector3 posKyotoR(-75.8, -21.33, 84.5);
  TVector3 normalKyotoR(1, 0, 0);
  fKyotoRPlane = genfit::SharedPlanePtr(new genfit::DetPlane(posKyotoR, normalKyotoR));

  TVector3 posKatana(-20, -21.33, 186.7);
  TVector3 normalKatana(0, 0, -1);
  fKatanaPlane = genfit::SharedPlanePtr(new genfit::DetPlane(posKatana, normalKatana));
}

void STGenfitTestE::SetTargetPlane(TVector3 position, TVector3 normal)
{
  fTargetPlane = genfit::SharedPlanePtr(new genfit::DetPlane(0.1*position, 0.1*normal));
}
void STGenfitTestE::SetMinIterations(Int_t value) { 
  fKalmanFitter -> setMinIterations(value);
}
void STGenfitTestE::SetMaxIterations(Int_t value) { 
  fKalmanFitter -> setMaxIterations(value);
}

void STGenfitTestE::Init()
{
  fHitClusterArray -> Delete();
  fGenfitTrackArray -> Delete();
}

genfit::Track* STGenfitTestE::FitTrack(STHelixTrack *helixTrack, Int_t pdg)
{
  fHitClusterArray -> Delete();
  genfit::TrackCand trackCand;

  UInt_t numHits = helixTrack -> GetNumStableClusters();
  if (numHits < 3) 
    return nullptr;

  auto clusterArray = helixTrack -> GetClusterArray();
  for (auto cluster : *clusterArray) {
    if (cluster -> IsStable() == false)
      continue;
    Int_t idx = fHitClusterArray->GetEntriesFast();
    new ((*fHitClusterArray)[idx]) STHitCluster(cluster);
    trackCand.addHit(fTPCDetID, idx);
  }

  STHitCluster *refCluster;
  for (auto cluster : *clusterArray) {
    if (cluster -> IsStable()) {
      refCluster = cluster;
      break;
    }
  }

  TVector3 posSeed = refCluster -> GetPosition();
  posSeed.SetMag(posSeed.Mag()/10.);

  TMatrixDSym covSeed(6);
  TMatrixD covMatrix = refCluster -> GetCovMatrix();
  for (Int_t iComp = 0; iComp < 3; iComp++)
    covSeed(iComp, iComp) = covMatrix(iComp, iComp)/100.;
  for (Int_t iComp = 3; iComp < 6; iComp++)
    covSeed(iComp, iComp) = covSeed(iComp - 3, iComp - 3);

  Double_t dip = helixTrack -> DipAngle();
  Double_t momSeedMag = helixTrack -> Momentum();
  TVector3 momSeed(0., 0., momSeedMag);
  momSeed.SetTheta(TMath::Pi()/2. - dip);

  trackCand.setCovSeed(covSeed);
  trackCand.setPosMomSeed(posSeed, momSeed, helixTrack -> Charge());

  genfit::Track *gfTrack = new ((*fGenfitTrackArray)[fGenfitTrackArray -> GetEntriesFast()]) genfit::Track(trackCand, *fMeasurementFactory);
  gfTrack -> addTrackRep(new genfit::RKTrackRep(pdg));
  Bool_t isFitted = ProcessTrack(gfTrack);

  if (!isFitted)
    return nullptr;

  return gfTrack;
}

Bool_t
STGenfitTestE::ProcessTrack(genfit::Track *gfTrack)
{
  Bool_t isFitted = kFALSE;

  genfit::RKTrackRep *trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);

  try {
    fKalmanFitter -> processTrackWithRep(gfTrack, trackRep, false);
    if (gfTrack -> getFitStatus(trackRep) -> isFitted())
      isFitted = kTRUE;
  } catch (genfit::Exception &e) {}

  return isFitted;
}

void 
STGenfitTestE::SetTrackParameters(STTrackCandidate *recoTrack, genfit::Track *gfTrack, TVector3 vertex)
{
  genfit::RKTrackRep *trackRep;
  genfit::MeasuredStateOnPlane fitState;
  genfit::FitStatus *fitStatus;

  try {
    trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
    fitState = gfTrack -> getFittedState();
    fitStatus = gfTrack -> getFitStatus(trackRep);
  } catch (genfit::Exception &e) {
    return;
  }

  if (fitStatus -> isFitted() == kFALSE || fitStatus -> isFitConverged() == kFALSE)
    return;

  if (vertex.Z() > -900) {
    try {
      trackRep -> extrapolateToPoint(fitState, .1*vertex);
    } catch (genfit::Exception &e) {
    }
  }

  TVector3 posReco(-999,-999,-999);
  TVector3 momReco(-999,-999,-999);

  TMatrixDSym covMat(6,6);
  Double_t bChi2 = 0, fChi2 = 0, bNdf = 0, fNdf = 0;

  Double_t pVal = 0;

  try {
    fitState.getPosMomCov(posReco, momReco, covMat);
    pVal = fitStatus -> getPVal();
    fKalmanFitter -> getChiSquNdf(gfTrack, trackRep, bChi2, fChi2, bNdf, fNdf);

  } catch (genfit::Exception &e) {
    return;
  }

  if (fNdf < 3)
    return;

  if (momReco.Z() < 0)
    momReco = -momReco;

  recoTrack -> SetVertex(posReco*10.);
  recoTrack -> SetMomentum(momReco*1000.);
  recoTrack -> SetPID(trackRep -> getPDG());
  recoTrack -> SetMass(fitState.getMass()*1000);
  recoTrack -> SetChi2(fChi2);
  recoTrack -> SetNDF(fNdf);
  recoTrack -> SetPVal(pVal);
  recoTrack -> SetCovSeed(covMat);

  Double_t charge = DetermineCharge(recoTrack, gfTrack);
  recoTrack -> SetCharge(charge);
  ProcessExtrapolation(recoTrack, gfTrack);
}

Int_t
STGenfitTestE::DetermineCharge(STTrackCandidate *recoTrack, genfit::Track *gfTrack)
{
  genfit::RKTrackRep *trackRep;
  genfit::MeasuredStateOnPlane fitState;

  try {
    trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
    fitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {
    return 0;
  }

  std::vector<genfit::TrackPoint *> pointArray = gfTrack -> getPointsWithMeasurement();
  Int_t numPoints = pointArray.size();

  genfit::STSpacepointMeasurement *point = (genfit::STSpacepointMeasurement *) pointArray.at(0) -> getRawMeasurement(0);
  try {
    trackRep -> extrapolateToMeasurement(fitState, point);
  } catch (genfit::Exception &e) {
    return 0;
  }
  TVector3 ref = fitState.getPos();
  ref.SetY(0);

  Double_t sum = 0;

  for (Int_t iPoint = 1; iPoint < numPoints-1; iPoint++)
  {
    genfit::STSpacepointMeasurement *point1 = (genfit::STSpacepointMeasurement *) pointArray.at(iPoint) -> getRawMeasurement(0);
    try {
      trackRep -> extrapolateToMeasurement(fitState, point1);
    } catch (genfit::Exception &e) {
      return 0;
    }
    TVector3 pos1 = fitState.getPos();

    genfit::STSpacepointMeasurement *point2 = (genfit::STSpacepointMeasurement *) pointArray.at(iPoint+1) -> getRawMeasurement(0);
    try {
      trackRep -> extrapolateToMeasurement(fitState, point2);
    } catch (genfit::Exception &e) {
      return 0;
    }
    TVector3 pos2 = fitState.getPos();

    pos1.SetY(0);
    pos2.SetY(0);
    sum += ((pos1-ref).Cross(pos2-ref)).Y();
  }

  return sum > 0 ? -1 : 1;
}

void 
STGenfitTestE::ProcessExtrapolation(STTrackCandidate *recoTrack, genfit::Track *gfTrack)
{
  genfit::RKTrackRep *trackRep;
  genfit::MeasuredStateOnPlane fitState;

  try {
    trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
    fitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {
    return;
  }

  TVector3 momTarget(-99999,-99999,-99999);
  TVector3 posTarget(-99999,-99999,-99999);
  try { 
    trackRep -> extrapolateToPlane(fitState, fTargetPlane); 
    momTarget = fitState.getMom();
    recoTrack -> SetBeamMomentum(momTarget);
    posTarget = fitState.getPos();
    recoTrack -> SetBeamVertex(posTarget*10.);
  } catch (genfit::Exception &e) {
  }

  TVector3 posKyotoL(-99999,-99999,-99999);
  try { 
    trackRep -> extrapolateToPlane(fitState, fKyotoLPlane); 
    posKyotoL = fitState.getPos();
    recoTrack -> SetKyotoLHit(posKyotoL*10.);
  } catch (genfit::Exception &e) {
  }

  TVector3 posKyotoR(-99999,-99999,-99999);
  try { 
    trackRep -> extrapolateToPlane(fitState, fKyotoRPlane); 
    posKyotoR = fitState.getPos();
    recoTrack -> SetKyotoRHit(posKyotoR*10.);
  } catch (genfit::Exception &e) {
  }

  TVector3 posKatana(-99999,-99999,-99999);
  try { 
    trackRep -> extrapolateToPlane(fitState, fKatanaPlane); 
    posKatana = fitState.getPos();
    recoTrack -> SetKatanaHit(posKatana*10.);
  } catch (genfit::Exception &e) {
  }
}

void
STGenfitTestE::VarifyClusters(genfit::Track *gfTrack, STHelixTrack *helixTrack)
{
  genfit::MeasuredStateOnPlane fitState;
  genfit::RKTrackRep *trackRep;
  try {
    trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
    fitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {}

  auto clusterArray = helixTrack -> GetClusterArray();
  auto numClusters = clusterArray -> size();

  for (auto iCluster = 0; iCluster < numClusters; iCluster++)
  {
    auto cluster = clusterArray -> at(iCluster);
    if (cluster -> IsStable() == false)
      continue;
    auto position = cluster -> GetPosition();
    try {
      trackRep -> extrapolateToPoint(fitState, .1*position);
      auto poca = 10*fitState.getPos();
      auto d = position - poca;
      d.SetY(0);
      if (d.Mag() > 5)
        cluster -> SetIsStable(false);
    } catch (genfit::Exception &e) {}
  }
}

Bool_t 
STGenfitTestE::CalculatedEdx(genfit::Track *gfTrack, STTrackCandidate *recoTrack, STHelixTrack *helixTrack)
{
  Int_t numPoints = helixTrack -> GetNumStableClusters();
  if (numPoints < 3)
    return kFALSE;

  genfit::RKTrackRep *trackRep;
  genfit::MeasuredStateOnPlane fitState;
  try {
    trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
    fitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {
    return kFALSE;
  }

  auto clusterArray = helixTrack -> GetClusterArray();
  auto numClusters = clusterArray -> size();

  STHitCluster *preCluster = clusterArray -> at(0);
  TVector3 position = preCluster -> GetPosition();

  Double_t dLength;
  try {
    dLength = trackRep -> extrapolateToPoint(fitState, .1*position);
  } catch (genfit::Exception &e) {
    recoTrack -> SetTotaldEdx(-1);
    return kFALSE;
  }
  dLength = 0;

  Double_t totaldE = 0;
  Double_t totaldx = 0;

  for (auto iCluster = 1; iCluster < numClusters; iCluster++)
  {
    auto curCluster = clusterArray -> at(iCluster);


    Double_t length = .5*std::abs(dLength);

    position = curCluster -> GetPosition();
    try {
      dLength = trackRep -> extrapolateToPoint(fitState, .1*position);
      curCluster -> SetPOCA(10*fitState.getPos());
    } catch (genfit::Exception &e) {
      recoTrack -> SetTotaldEdx(-1);
      return kFALSE;
    }

    length += .5*std::abs(dLength);
    preCluster -> SetLength(10*length);

    if (preCluster -> IsStable())
    {
      Double_t dE = preCluster -> GetCharge();
      Double_t dx = preCluster -> GetLength();

      Double_t dEdx = dE/dx;
      recoTrack -> AdddEdx(dEdx);

      totaldE += dE;
      totaldx += dx;
    }

    preCluster = curCluster;
  }

  recoTrack -> SetTrackLength(totaldx);
  recoTrack -> SetTotaldEdx(totaldE/totaldx);

  return kTRUE;
}

Bool_t 
STGenfitTestE::CalculatedEdx2(genfit::Track *gfTrack, STTrackCandidate *recoTrack, STHelixTrack *helixTrack)
{
  Int_t numPoints = helixTrack -> GetNumStableClusters();
  if (numPoints < 3)
    return kFALSE;

  genfit::RKTrackRep *trackRep;
  genfit::MeasuredStateOnPlane fitState;
  try {
    trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
    fitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {
    return kFALSE;
  }

  auto clusterArray = helixTrack -> GetClusterArray();
  auto numClusters = clusterArray -> size();

  Double_t totaldE = 0;
  Double_t totaldx = 0;

  for (auto iCluster = 0; iCluster < numClusters; iCluster++)
  {
    auto cluster = clusterArray -> at(iCluster);
    if (!cluster -> IsStable())
      continue;

    auto row = cluster -> GetRow();
    auto layer = cluster -> GetLayer();
    auto position = cluster -> GetPosition();

    Bool_t buildByLayer = true;
    if (layer == -1)
      buildByLayer = false;

    TVector3 pointRef1 = .1*position;
    TVector3 pointRef2 = .1*position;
    TVector3 normalRef(0,0,0);
    if (buildByLayer) {
      normalRef = TVector3(0,0,1);
      pointRef1.SetZ(.1*(layer*12.));
      pointRef2.SetZ(.1*((layer+1)*12.));
    } else {
      normalRef = TVector3(1,0,0);
      pointRef1.SetX(.1*(row*8.-432.));
      pointRef2.SetX(.1*((row+1)*8.-432.));
    }

    genfit::SharedPlanePtr referencePlane1 = genfit::SharedPlanePtr(new genfit::DetPlane(pointRef1, normalRef));
    genfit::SharedPlanePtr referencePlane2 = genfit::SharedPlanePtr(new genfit::DetPlane(pointRef2, normalRef));

    try {
      trackRep -> extrapolateToPlane(fitState, referencePlane1);
      Double_t l2 = 10 * trackRep -> extrapolateToPlane(fitState, referencePlane2);
      cluster -> SetLength(std::abs(l2));
      cluster -> SetPOCA(10*fitState.getPos());
    } catch (genfit::Exception &e) {
      cluster -> SetIsStable(false);
      continue;
    }

    Double_t dE = cluster -> GetCharge();
    Double_t dx = cluster -> GetLength();

    if (dx > 20) {
      cluster -> SetIsStable(false);
      continue;
    }

    Double_t dEdx = dE/dx;
    recoTrack -> AdddEdx(dEdx);

    totaldE += dE;
    totaldx += dx;
  }

  recoTrack -> SetTrackLength(totaldx);
  recoTrack -> SetTotaldEdx(totaldE/totaldx);

  return kTRUE;
}
