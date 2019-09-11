#include "STGenfitTest.hh"

// SPiRITROOT classes
#include "STGenfitTask.hh"
#include "STRiemannHit.hh"
#include "STTrackCandidate.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"
#include "STGFBField.hh"

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

#define JUSTPROTON

ClassImp(STGenfitTest)

STGenfitTest::STGenfitTest()
:STGenfitTest(kTRUE)
{
}

STGenfitTest::STGenfitTest(Bool_t loadSamurai)
{
  fTPCDetID = 0;
  fCurrentDirection = 1;

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

  STDatabasePDG* db = STDatabasePDG::Instance();
#ifdef JUSTPROTON
  fPDGCandidateArray = new std::vector<Int_t>;
  fPDGCandidateArray -> push_back(2212);
#else
  fPDGCandidateArray = db -> GetPDGCandidateArray();
#endif

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

void 
STGenfitTest::Init()
{
  fHitClusterArray -> Delete();
  fGenfitTrackArray -> Delete();
}

void STGenfitTest::SetTargetPlane(TVector3 position, TVector3 normal)
{
  fTargetPlane = genfit::SharedPlanePtr(new genfit::DetPlane(0.1*position, 0.1*normal));
}

void STGenfitTest::SetMinIterations(Int_t value) { 
  fKalmanFitter -> setMinIterations(value);
}
void STGenfitTest::SetMaxIterations(Int_t value) { 
  fKalmanFitter -> setMaxIterations(value);
}

genfit::Track*
STGenfitTest::FitTrack(STTrack *recoTrack,
                       STEvent *event, 
                       STRiemannTrack *riemannTrack)
{
  fHitClusterArray -> Delete();
  genfit::TrackCand trackCand;

  UInt_t numHits = riemannTrack -> GetNumHits();
  if (numHits < 3) 
    return nullptr;

  for (UInt_t iHit = 0; iHit < numHits; iHit++) 
  {
    Int_t id = riemannTrack -> GetHit(iHit) -> GetHit() -> GetClusterID();
    STHitCluster *cluster = event -> GetCluster(id);

    new ((*fHitClusterArray)[iHit]) STHitCluster(*cluster);
    trackCand.addHit(fTPCDetID, iHit);

    recoTrack -> AddHitID(id);
  }

  // Initial parameter setting
  // TODO : improve initial parameter
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

  genfit::Track *genfitTrack = new ((*fGenfitTrackArray)[fGenfitTrackArray -> GetEntriesFast()]) genfit::Track(trackCand, *fMeasurementFactory);

  for (Int_t pdg : *fPDGCandidateArray)  {
    if (pdg == 11 || pdg == -211)
      trackCand.setPosMomSeed(posSeed, momSeed, -1);
    else
      trackCand.setPosMomSeed(posSeed, momSeed, 1);
    genfitTrack -> addTrackRep(new genfit::RKTrackRep(pdg));
  }

  Bool_t isFitted = ProcessTrack(genfitTrack);

  if (!isFitted)
    return nullptr;

  SetTrackParameters(recoTrack, genfitTrack, event, riemannTrack);

  return genfitTrack;
}

Bool_t 
STGenfitTest::ProcessTrack(genfit::Track *genfitTrack)
{
  Bool_t isFitted = kFALSE;

  UInt_t numReps = genfitTrack -> getNumReps();
  for (UInt_t iRep = 0; iRep < numReps; iRep++)
  {
    genfitTrack -> setCardinalRep(iRep);
    genfit::RKTrackRep *trackRep = (genfit::RKTrackRep *) genfitTrack -> getTrackRep(iRep);
    try {
      fKalmanFitter -> processTrackWithRep(genfitTrack, trackRep, false);
      if (genfitTrack -> getFitStatus(trackRep) -> isFitted())
        isFitted = kTRUE;
    } catch (genfit::Exception &e) {}
  }

  return isFitted;
}

void 
STGenfitTest::SetTrackParameters(STTrack *recoTrack, genfit::Track *genfitTrack, STEvent *event, STRiemannTrack *riemannTrack)
{
  fRecoTrackCand = nullptr;

  Double_t bestChi2 = 1.e10;
  STTrackCandidate *bestTrackCandidate = nullptr;

  genfit::RKTrackRep *CardinalTrackRep = nullptr;
  try {
    genfitTrack -> determineCardinalRep();
    CardinalTrackRep = (genfit::RKTrackRep *) genfitTrack -> getCardinalRep();
  } catch (genfit::Exception &e) {}

  UInt_t numReps = genfitTrack -> getNumReps();
  for (UInt_t iRep = 0; iRep < numReps; iRep++)
  {
    Int_t idebug = 0;
    fCurrentTrackRep = (genfit::RKTrackRep *) genfitTrack -> getTrackRep(iRep);
    try {
      fCurrentFitState = genfitTrack -> getFittedState();
      fCurrentFitStatus = genfitTrack -> getFitStatus(fCurrentTrackRep);
    } catch (genfit::Exception &e) {
      continue;
    }
    if (fCurrentFitStatus -> isFitted() == kFALSE || fCurrentFitStatus -> isFitConverged() == kFALSE)
      continue;

    TVector3 posReco(-99999,-99999,-99999);
    TVector3 momReco(-99999,-99999,-99999);
    TMatrixDSym covMat(6,6);
    Double_t bChi2 = 0, fChi2 = 0, bNdf = 0, fNdf = 0;

    try {
      fCurrentFitState.getPosMomCov(posReco, momReco, covMat);
      fKalmanFitter -> getChiSquNdf(genfitTrack, fCurrentTrackRep, bChi2, fChi2, bNdf, fNdf);
    } catch (genfit::Exception &e) {
      continue;
    }

    if (fNdf < 3)
      continue;

    Double_t totaldEdxRiemann = 0;
    Double_t totalLength = 0;

    CalculatedEdxFromRiemann(event, riemannTrack, totalLength, totaldEdxRiemann);

#ifdef JUSTPROTON
    recoTrack -> SetVertex(posReco*10.);
    recoTrack -> SetMomentum(momReco*1000.);
    recoTrack -> SetPID(fCurrentTrackRep -> getPDG());
    recoTrack -> SetMass(fCurrentFitState.getMass()*1000);
    recoTrack -> SetCharge(fCurrentFitState.getCharge());
    recoTrack -> SetChi2(fChi2);
    recoTrack -> SetNDF(fNdf);
    recoTrack -> SetTrackLength(totalLength);
    recoTrack -> SetRiemanndEdx(totaldEdxRiemann);

    FindAndSetExtrapolation(recoTrack);
    CalculatedEdx(recoTrack, genfitTrack);
#else
    fRecoTrackCand = new STTrackCandidate();
    fRecoTrackCand -> SetVertex(posReco*10.);
    fRecoTrackCand -> SetMomentum(momReco*1000.);
    fRecoTrackCand -> SetPID(fCurrentTrackRep -> getPDG());
    fRecoTrackCand -> SetMass(fCurrentFitState.getMass()*1000);
    fRecoTrackCand -> SetCharge(fCurrentFitState.getCharge());
    fRecoTrackCand -> SetChi2(fChi2);
    fRecoTrackCand -> SetNDF(fNdf);
    fRecoTrackCand -> SetTrackLength(totalLength);
    fRecoTrackCand -> SetRiemanndEdx(totaldEdxRiemann);

    FindAndSetExtrapolation(fRecoTrackCand);
    CalculatedEdx(fRecoTrackCand, genfitTrack);
    recoTrack -> AddTrackCandidate(fRecoTrackCand);

    if (CardinalTrackRep != nullptr && CardinalTrackRep == fCurrentTrackRep) {
      bestTrackCandidate = fRecoTrackCand;
    }
    else if (fChi2 < bestChi2) {
      bestChi2 = fChi2;
      bestTrackCandidate = fRecoTrackCand;
    }
#endif
    recoTrack -> SetIsFitted();
  }

#ifndef JUSTPROTON
  if (bestTrackCandidate != nullptr) {
    recoTrack -> SelectTrackCandidate(bestTrackCandidate);
  } else if(fRecoTrackCand != nullptr)
    recoTrack -> SelectTrackCandidate(fRecoTrackCand);
#endif
}

void 
STGenfitTest::FindAndSetExtrapolation(STTrackCandidate *recoTrackCand)
{
  TVector3 momTarget(-99999,-99999,-99999);
  TVector3 posTarget(-99999,-99999,-99999);
  try { 
    fCurrentTrackRep -> extrapolateToPlane(fCurrentFitState, fTargetPlane); 
    momTarget = fCurrentFitState.getMom();
    recoTrackCand -> SetBeamMomentum(momTarget);
    posTarget = fCurrentFitState.getPos();
    recoTrackCand -> SetBeamVertex(posTarget*10.);
  } catch (genfit::Exception &e) {
  }

  TVector3 posKyotoL(-99999,-99999,-99999);
  try { 
    fCurrentTrackRep -> extrapolateToPlane(fCurrentFitState, fKyotoLPlane); 
    posKyotoL = fCurrentFitState.getPos();
    recoTrackCand -> SetKyotoLHit(posKyotoL*10.);
  } catch (genfit::Exception &e) {
  }

  TVector3 posKyotoR(-99999,-99999,-99999);
  try { 
    fCurrentTrackRep -> extrapolateToPlane(fCurrentFitState, fKyotoRPlane); 
    posKyotoR = fCurrentFitState.getPos();
    recoTrackCand -> SetKyotoRHit(posKyotoR*10.);
  } catch (genfit::Exception &e) {
  }

  TVector3 posKatana(-99999,-99999,-99999);
  try { 
    fCurrentTrackRep -> extrapolateToPlane(fCurrentFitState, fKatanaPlane); 
    posKatana = fCurrentFitState.getPos();
    recoTrackCand -> SetKatanaHit(posKatana*10.);
  } catch (genfit::Exception &e) {
  }

}

Bool_t
STGenfitTest::SetTrack(STEvent *event, STTrack *recoTrack)
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
  if (pTot == 0) 
    return kFALSE;

  Double_t pZ = recoTrack -> GetPz();
  TVector3 momSeed(0., 0., pTot);
  momSeed.SetTheta(TMath::Pi()/2. - TMath::ACos(pZ/pTot));

  trackCand.setCovSeed(covSeed);
  trackCand.setPosMomSeed(posSeed, momSeed, recoTrack -> GetCharge());

  Bool_t fitted = kFALSE;

  Int_t pdg = recoTrack -> GetPID();
  fCurrentTrackRep = new genfit::RKTrackRep(pdg); 

  fGenfitTrackArray -> Delete();
  genfit::Track *genfitTrack = new ((*fGenfitTrackArray)[0]) genfit::Track(trackCand, *fMeasurementFactory, fCurrentTrackRep);

  try {
    fKalmanFitter -> processTrack(genfitTrack);
  } catch (genfit::Exception &e) {
    return kFALSE;
  }

  try {
    fCurrentFitState = genfitTrack -> getFittedState();
  } catch (genfit::Exception &e) {
    return kFALSE;
  }

  try {
    fCurrentTrackRep -> extrapolateToPlane(fCurrentFitState, fTargetPlane);
  } catch (genfit::Exception &e) {
  }

  fitted = kTRUE;

  fCurrentDirection = 1;
  if (fCurrentTrackRep -> getDir(fCurrentFitState).Z() < 0)
    fCurrentDirection = -1;

  return fitted;
}

Bool_t
STGenfitTest::SetTrack(TClonesArray *array, STTrack *recoTrack)
{
  fHitClusterArray -> Delete();
  std::vector<Int_t> *hitIDArray = recoTrack -> GetHitIDArray();
  genfit::TrackCand trackCand;

  UInt_t numHits = hitIDArray -> size(); 
  for (UInt_t iHit = 0; iHit < numHits; iHit++) 
  {
    STHitCluster *cluster = (STHitCluster *) array -> At(hitIDArray -> at(iHit));
    new ((*fHitClusterArray)[iHit]) STHitCluster(*cluster);
    trackCand.addHit(fTPCDetID, iHit);
  }

  STHitCluster *firstCluster = (STHitCluster *) array -> At(hitIDArray -> at(0));

  TVector3 posSeed = firstCluster -> GetPosition();
  posSeed.SetMag(posSeed.Mag()/10.);

  TMatrixDSym covSeed(6);
  TMatrixD covMatrix = firstCluster -> GetCovMatrix();
  for (Int_t iComp = 0; iComp < 3; iComp++)
    covSeed(iComp, iComp) = covMatrix(iComp, iComp)/100.;
  for (Int_t iComp = 3; iComp < 6; iComp++)
    covSeed(iComp, iComp) = covSeed(iComp - 3, iComp - 3);

  Double_t pTot = recoTrack -> GetP();
  if (pTot == 0) 
    return kFALSE;

  Double_t pZ = recoTrack -> GetPz();
  TVector3 momSeed(0., 0., pTot);
  momSeed.SetTheta(TMath::Pi()/2. - TMath::ACos(pZ/pTot));

  trackCand.setCovSeed(covSeed);
  trackCand.setPosMomSeed(posSeed, momSeed, recoTrack -> GetCharge());

  Bool_t fitted = kFALSE;

  Int_t pdg = recoTrack -> GetPID();
  fCurrentTrackRep = new genfit::RKTrackRep(pdg); 

  fGenfitTrackArray -> Delete();
  genfit::Track *genfitTrack = new ((*fGenfitTrackArray)[0]) genfit::Track(trackCand, *fMeasurementFactory, fCurrentTrackRep);

  try {
    fKalmanFitter -> processTrackWithRep(genfitTrack, fCurrentTrackRep, false);
  } catch (genfit::Exception &e) {
    return kFALSE;
  }

  try {
    fCurrentFitState = genfitTrack -> getFittedState();
  } catch (genfit::Exception &e) {
    return kFALSE;
  }

  try {
    fCurrentTrackRep -> extrapolateToPlane(fCurrentFitState, fTargetPlane);
  } catch (genfit::Exception &e) {
  }

  fitted = kTRUE;

  fCurrentDirection = 1;
  if (fCurrentTrackRep -> getDir(fCurrentFitState).Z() < 0)
    fCurrentDirection = -1;

  return fitted;
}

Bool_t 
STGenfitTest::ExtrapolateTrack(Double_t distance, TVector3 &position)
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

Bool_t 
STGenfitTest::CalculatedEdx(STTrack *recoTrack, genfit::Track *genfitTrack)
{
  fCurrentFitState = genfitTrack -> getFittedState();

  std::vector<genfit::TrackPoint *> pointArray = genfitTrack -> getPointsWithMeasurement();

  Int_t numPoints = pointArray.size();
  if (numPoints < 3)
    return kFALSE;

  genfit::STSpacepointMeasurement *point;

  Double_t dEdx = 0;
  Double_t dEdxTotal = 0;

  Double_t charge = 0;
  Double_t lengthTotal = 0;
  Double_t lengthNextHalf = 0;

  Int_t idx = 0;
  point = (genfit::STSpacepointMeasurement *) pointArray.at(idx) -> getRawMeasurement(0);
  idx++;

  try {
    fCurrentTrackRep -> extrapolateToMeasurement(fCurrentFitState, point);
  } catch (genfit::Exception &e) {
    recoTrack -> SetTotaldEdx(-1);
    return kFALSE;
  }

  charge = point -> GetCharge();
  charge = charge/2.;

  while (idx < numPoints)
  {
    point = (genfit::STSpacepointMeasurement *) pointArray.at(idx) -> getRawMeasurement(0);
    idx++;

    try {
      lengthNextHalf = (fCurrentTrackRep -> extrapolateToMeasurement(fCurrentFitState, point))/2.;
    } catch (genfit::Exception &e) {
      recoTrack -> SetTotaldEdx(-1);
      return kFALSE;
    }

    lengthTotal += lengthNextHalf;

    dEdx = charge/lengthTotal/10.;
    dEdxTotal += dEdx;
    recoTrack -> AdddEdx(dEdx);

    lengthTotal = lengthNextHalf;
    charge = point -> GetCharge();
  }

  charge = charge/2.;
  dEdx = charge/lengthTotal/10.;
  dEdxTotal += dEdx;
  recoTrack -> AdddEdx(dEdx);

  dEdxTotal = dEdxTotal/numPoints;
  recoTrack -> SetTotaldEdx(dEdxTotal);

  return kTRUE;
}

Bool_t
STGenfitTest::CalculatedEdxFromRiemann(STEvent *event, STRiemannTrack *track, Double_t &totalLength, Double_t &totaldEdx)
{
  const std::vector<STRiemannHit *> *hitArray = track -> GetHits();

  totalLength = -1;
  totaldEdx = -1;

  Int_t numHits = hitArray -> size();
  if(numHits < 3) 
    return kFALSE; 

  totalLength = 0;
  Double_t totalEloss = 0;

  for (Int_t iHit = 0; iHit < numHits; iHit++) {

    STRiemannHit *hit = hitArray -> at(iHit);
    //gets hit in rotated frame, rotated frame is the STRiemann frame.
    STHit *hitrotated = (STHit *)hit->GetHit();
    Int_t clusID = hitrotated -> GetClusterID();

    //STHitCluster is in the TPC normal coordinate frame
    STHitCluster *hitcluster = (STHitCluster *) event->GetCluster(clusID);

    //Position vectors in the Helix frame
    TVector3 pos_0; // k-1 hit
    TVector3 pos_k; // current hit, k
    TVector3 pos_1; // k+1 hit

    TVector3 dl; // difference between pos_0 and pos_k vectors
    TVector3 dl_1; // difference between pos_k and pos_1 vectors

    TVector3 dir; // dir vector of k_th position 
    TVector3 temp; // junk dir vector

    //calculation of path length with GetPosDirOnHelix() function
    if((iHit+1) < numHits && (iHit-1) > 0){ //taking care of the end points

      // These directions are in the rotated frame of STRiemannHit
      // But the path lengths should not change even in a rotated frame
      // Do not use the pos dir vectors from these to calculate angles

      track->GetPosDirOnHelix(iHit-1,pos_0,temp);
      track->GetPosDirOnHelix(iHit,pos_k,dir);
      track->GetPosDirOnHelix(iHit+1,pos_1,temp);

      dl=pos_k-pos_0;
      dl_1=pos_1-pos_k;

      Double_t mag = dl.Mag();
      Double_t mag_1 = dl_1.Mag();

      Double_t clusterlength = (mag+mag_1)/2; // half the distance from k-1 to k and from k to k+1

      totalLength += clusterlength; // total distance along track for mean dE/dx 
      totalEloss += hitcluster->GetCharge();
    }
  }

  if (totalLength <= 0) {
    totalLength = -1;
    totaldEdx = -1;
  } else
    totaldEdx = totalEloss/totalLength;

  return kTRUE;
}
