#include "STGenfitTrajectory.hh"

// SPiRITROOT classes
#include "STGenfitTask.hh"
#include "STRiemannHit.hh"
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

ClassImp(STGenfitTrajectory)

STGenfitTrajectory::STGenfitTrajectory()
{
  fTPCDetID = 0;
  fCurrentDirection = 1;

  fMinIterations = 5;
  fMaxIterations = 20;

  fKalmanFitter = new genfit::DAF();
  fKalmanFitter -> setMinIterations(fMinIterations);
  fKalmanFitter -> setMaxIterations(fMaxIterations);

  fHitClusterArray = new TClonesArray("STHitCluster");
  fGenfitTrackArray = new TClonesArray("genfit::Track");

  fMeasurementProducer = new genfit::MeasurementProducer<STHitCluster, genfit::STSpacepointMeasurement>(fHitClusterArray);
  fMeasurementFactory = new genfit::MeasurementFactory<genfit::AbsMeasurement>();
  fMeasurementFactory -> addProducer(fTPCDetID, fMeasurementProducer);

  genfit::FieldManager::getInstance() -> init(new genfit::ConstField(0., 5., 0.));
  genfit::MaterialEffects *materialEffects = genfit::MaterialEffects::getInstance();
  materialEffects -> init(new genfit::TGeoMaterialInterface());
  //materialEffects -> setNoiseCoulomb(kFALSE);
  //materialEffects -> setNoiseBrems(kFALSE);
  //materialEffects -> setNoiseBetheBloch(kFALSE);

  STDatabasePDG* db = STDatabasePDG::Instance();
  fPDGCandidateArray = db -> GetPDGCandidateArray();

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

Bool_t 
STGenfitTrajectory::SetTrack(STTrack *recoTrack,
                             STEvent *event, 
                             STRiemannTrack *riemannTrack)
{
  fHitClusterArray -> Delete();
  genfit::TrackCand trackCand;

  UInt_t numHits = riemannTrack -> GetNumHits();
  for (UInt_t iHit = 0; iHit < numHits; iHit++) 
  {
    STRiemannHit *hit = riemannTrack -> GetHit(iHit);
    Int_t clusterID = hit -> GetHit() -> GetClusterID();
    STHitCluster cluster = event -> GetCluster(clusterID);
    new ((*fHitClusterArray)[iHit]) STHitCluster(&cluster);
    trackCand.addHit(fTPCDetID, iHit);
    recoTrack -> AddHitID(clusterID);
  }

  {
    STRiemannHit *hit = riemannTrack -> GetFirstHit();
    STHitCluster *firstCluster = event -> GetCluster(hit -> GetHit() -> GetClusterID());
    TVector3 posSeed = firstCluster -> GetPosition();
    posSeed.SetMag(posSeed.Mag()/10.);

    TMatrixDSym covSeed(6);
    TMatrixD covMatrix = firstCluster -> GetCovMatrix();
    for (Int_t iComp = 0; iComp < 3; iComp++) 
      covSeed(iComp, iComp) = covMatrix(iComp, iComp)/100.;
    for (Int_t iComp = 3; iComp < 6; iComp++) 
      covSeed(iComp, iComp) = covSeed(iComp - 3, iComp - 3);

    Double_t dip = riemannTrack -> GetDip();
    Double_t momSeedMag = riemannTrack -> GetMom(5.);
    TVector3 momSeed(0., 0., momSeedMag);
    momSeed.SetTheta(TMath::Pi()/2. - dip);

    trackCand.setCovSeed(covSeed);
    trackCand.setPosMomSeed(posSeed, momSeed, 1);
  }

  //fCurrentTrackRep = new genfit::RKTrackRep(2212);

  fGenfitTrackArray -> Delete();
  genfit::Track *genfitTrack = new ((*fGenfitTrackArray)[0]) genfit::Track(trackCand, *fMeasurementFactory);

  //for (Int_t pdg : *fPDGCandidateArray) 
    //genfitTrack -> addTrackRep(new genfit::RKTrackRep(pdg));

  fCurrentTrackRep = new genfit::RKTrackRep(2212);
  genfitTrack -> addTrackRep(fCurrentTrackRep);

  Bool_t fitted = ProcessTrack(genfitTrack);

  return fitted;
}

Bool_t STGenfitTrajectory::ProcessTrack(genfit::Track *genfitTrack)
{
  try {
    fKalmanFitter -> processTrack(genfitTrack);
    fCurrentFitState = genfitTrack -> getFittedState();
    fCurrentTrackRep -> extrapolateToPlane(fCurrentFitState, fTargetPlane); 
  } catch (genfit::Exception &e) {
    return kFALSE;
  }

  return kTRUE;
}


Bool_t STGenfitTrajectory::SetTrack(STEvent *event, STTrack *recoTrack)
{
  fHitClusterArray -> Delete();
  std::vector<Int_t> *hitIDArray = recoTrack -> GetHitIDArray();
  genfit::TrackCand trackCand;

  UInt_t numHits = hitIDArray -> size(); 
  for (UInt_t iHit = 0; iHit < numHits; iHit++) 
  {
    STHitCluster *cluster = event -> GetCluster(hitIDArray -> at(iHit));
    new ((*fHitClusterArray)[iHit]) STHitCluster(*cluster);
    trackCand.addHit(fTPCDetID, iHit);
  }
  {
    STHitCluster *firstCluster = event -> GetCluster(hitIDArray -> at(0));

    TVector3 posSeed = firstCluster -> GetPosition();
    posSeed.SetMag(posSeed.Mag()/10.);

    TMatrixDSym covSeed(6);
    TMatrixD covMatrix = firstCluster -> GetCovMatrix();
    for (Int_t iComp = 0; iComp < 3; iComp++) 
      covSeed(iComp, iComp) = covMatrix(iComp, iComp)/100.;
    for (Int_t iComp = 3; iComp < 6; iComp++) 
      covSeed(iComp, iComp) = covSeed(iComp - 3, iComp - 3);

    Double_t pTot = recoTrack -> GetP();
    Double_t pZ = recoTrack -> GetPz();
    TVector3 momSeed(0., 0., pTot);
    momSeed.SetTheta(TMath::Pi()/2. - TMath::ACos(pZ/pTot));

    trackCand.setCovSeed(covSeed);
    trackCand.setPosMomSeed(posSeed, momSeed, 1);
  }

  Bool_t fitted = kFALSE;

  Int_t pdg = recoTrack -> GetPID();
  {
    // TrackRep Is deleted later in side the genfit::Track
    fCurrentTrackRep = new genfit::RKTrackRep(pdg); 

    fGenfitTrackArray -> Delete();
    genfit::Track *genfitTrack = new ((*fGenfitTrackArray)[0]) genfit::Track(trackCand, *fMeasurementFactory, fCurrentTrackRep);

    try {
      fKalmanFitter -> processTrack(genfitTrack);
      fCurrentFitState = genfitTrack -> getFittedState();
      fCurrentTrackRep -> extrapolateToPlane(fCurrentFitState, fTargetPlane); 
    } catch (genfit::Exception &e) {
      return kFALSE;
    }
    fitted = kTRUE;

    fCurrentDirection = 1;
    if (fCurrentTrackRep -> getDir(fCurrentFitState).Z() < 0)
      fCurrentDirection = -1;
  }

  return fitted;
}

Bool_t STGenfitTrajectory::ExtrapolateTrack(Double_t distance, TVector3 &position)
{
  Double_t d = fCurrentDirection * 0.1 * distance;

  try { 
    fCurrentTrackRep -> extrapolateBy(fCurrentFitState, d); 
  } catch (genfit::Exception &e) { 
    return kFALSE; 
  }

  position = fCurrentFitState.getPos();
  return kTRUE;
}
