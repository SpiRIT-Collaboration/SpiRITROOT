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

genfit::Track* STGenfitTestE::FitTrack(STTrackCandidate *recoTrack, TClonesArray *hitArray, STHelixTrack *helixTrack, Int_t pdg)
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

  STHitCluster *firstCluster;
  for (auto cluster : *clusterArray) {
    if (cluster -> IsStable()) {
      firstCluster = cluster;
      break;
    }
  }
  TVector3 posSeed = firstCluster -> GetPosition();
  posSeed.SetMag(posSeed.Mag()/10.);

  TMatrixDSym covSeed(6);
  TMatrixD covMatrix = firstCluster -> GetCovMatrix();
  for (Int_t iComp = 0; iComp < 3; iComp++)
    covSeed(iComp, iComp) = covMatrix(iComp, iComp)/100.;
  for (Int_t iComp = 3; iComp < 6; iComp++)
    covSeed(iComp, iComp) = covSeed(iComp - 3, iComp - 3);

  Double_t dip = helixTrack -> DipAngle();
  Double_t momSeedMag = helixTrack -> Momentum();
  TVector3 momSeed(0., 0., momSeedMag);
  momSeed.SetTheta(TMath::Pi()/2. - dip);

  trackCand.setCovSeed(covSeed);
  if (pdg == 11 || pdg == -211)
    trackCand.setPosMomSeed(posSeed, momSeed, -1);
  else
    trackCand.setPosMomSeed(posSeed, momSeed, 1);

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
STGenfitTestE::SetTrack(STTrackCandidate *recoTrack, genfit::Track *gfTrack)
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

  TVector3 posReco(-99999,-99999,-99999);
  TVector3 momReco(-99999,-99999,-99999);

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

  recoTrack -> SetVertex(posReco*10.);
  recoTrack -> SetMomentum(momReco*1000.);
  recoTrack -> SetPID(trackRep -> getPDG());
  recoTrack -> SetMass(fitState.getMass()*1000);
  recoTrack -> SetCharge(fitState.getCharge());
  recoTrack -> SetChi2(fChi2);
  recoTrack -> SetNDF(fNdf);
  recoTrack -> SetPVal(pVal);

  ProcessExtrapolation(recoTrack, gfTrack);
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

Bool_t 
STGenfitTestE::CalculatedEdx(genfit::Track *gfTrack, STTrack *recoTrack, STHelixTrack *helixTrack)
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

  recoTrack -> SetTotaldEdx(totaldE/totaldx);

  return kTRUE;
}
