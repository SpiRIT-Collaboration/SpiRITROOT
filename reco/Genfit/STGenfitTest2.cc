#include "STGenfitTest2.hh"

// SPiRITROOT classes
#include "STGenfitTask.hh"
#include "STTrackCandidate.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"
#include "STdEdxPoint.hh"
#include "STGeoLine.hh"

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

ClassImp(STGenfitTest2)

STGenfitTest2::STGenfitTest2()
:STGenfitTest2(true)
{
}

STGenfitTest2::STGenfitTest2(bool loadSamurai)
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
    genfit::FieldManager::getInstance() -> init(new STGFBField("samurai_field_map","A"));
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

  TVector3 posNeuland(436.21, -21.33, 826.92);
  TVector3 normalNeuland(-0.494, 0, -0.870);
  fNeulandPlane = genfit::SharedPlanePtr(new genfit::DetPlane(posNeuland, normalNeuland));
}

void STGenfitTest2::SetMinIterations(Int_t value) { fKalmanFitter -> setMinIterations(value); }
void STGenfitTest2::SetMaxIterations(Int_t value) { fKalmanFitter -> setMaxIterations(value); }

void STGenfitTest2::Init()
{
  fHitClusterArray -> Delete();
  fGenfitTrackArray -> Delete();
}

genfit::Track* STGenfitTest2::FitTrack(STHelixTrack *helixTrack, Int_t pdg)
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
  if (refCluster == nullptr)
    return nullptr;

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

  genfit::RKTrackRep *trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);

  try {
    fKalmanFitter -> processTrackWithRep(gfTrack, trackRep, false);
  } catch (genfit::Exception &e) {}

  genfit::FitStatus *fitStatus;
  try {
    fitStatus = gfTrack -> getFitStatus(trackRep);
  } catch (genfit::Exception &e) {
    return nullptr;
  }

  if (fitStatus -> isFitted() == false || fitStatus -> isFitConverged() == false)
    return nullptr;

  genfit::MeasuredStateOnPlane fitState;
  try {
    fitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {}

  /*
  for (auto cluster : *clusterArray) {
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
  */

  return gfTrack;
}

genfit::Track* STGenfitTest2::FitTrackWithVertex(STHelixTrack *helixTrack, STHitCluster *vertex, Int_t pdg)
{
  fHitClusterArray -> Delete();
  genfit::TrackCand trackCand;

  UInt_t numHits = helixTrack -> GetNumStableClusters();
  if (numHits < 3) 
    return nullptr;

//  new ((&fHitClusterArray)[0]) STHitCluster(vertex);
//  trackCand.addHit(fTPCDetID, 0);

  auto clusterArray = helixTrack -> GetClusterArray();
  for (auto cluster : *clusterArray) {
    if (cluster -> IsStable() == false)
      continue;
    
    Int_t idx = fHitClusterArray->GetEntriesFast();
    new ((*fHitClusterArray)[idx]) STHitCluster(cluster);
    trackCand.addHit(fTPCDetID, idx);
  }

  STHitCluster *refCluster = vertex;
/*
  STHitCluster *refCluster;
  for (auto cluster : *clusterArray) {
    if (cluster -> IsStable()) {
      refCluster = cluster;
      break;
    }
  }
  if (refCluster == nullptr)
    return nullptr;
    */

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

  genfit::RKTrackRep *trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);

  try {
    fKalmanFitter -> processTrackWithRep(gfTrack, trackRep, false);
  } catch (genfit::Exception &e) {}

  genfit::FitStatus *fitStatus;
  try {
    fitStatus = gfTrack -> getFitStatus(trackRep);
  } catch (genfit::Exception &e) {
    return nullptr;
  }

  if (fitStatus -> isFitted() == false || fitStatus -> isFitConverged() == false)
    return nullptr;

  genfit::MeasuredStateOnPlane fitState;
  try {
    fitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {}

  for (auto cluster : *clusterArray) {
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

  return gfTrack;
}

genfit::Track* STGenfitTest2::SetTrack(STRecoTrack *recoTrack, TClonesArray *clusterArray)
{
  fHitClusterArray -> Delete();
  genfit::TrackCand trackCand;

  auto clusterIDArray = recoTrack -> GetClusterIDArray();
  Int_t idx = 0;
  for (auto id : *clusterIDArray) {
    auto cluster = (STHitCluster *) clusterArray -> At(id);
    new ((*fHitClusterArray)[idx]) STHitCluster(*cluster);
    trackCand.addHit(fTPCDetID, idx);
    idx++;
  }

  STHitCluster *refCluster;
  for (auto id : *clusterIDArray) {
    auto cluster = (STHitCluster *) clusterArray -> At(id);
    if (cluster -> IsStable()) {
      refCluster = cluster;
      break;
    }
  }

  if (refCluster == nullptr)
    return nullptr;

  TVector3 posSeed = refCluster -> GetPosition();
  posSeed.SetMag(posSeed.Mag()/10.);

  TMatrixDSym covSeed(6);
  TMatrixD covMatrix = refCluster -> GetCovMatrix();
  for (Int_t iComp = 0; iComp < 3; iComp++)
    covSeed(iComp, iComp) = covMatrix(iComp, iComp)/100.;
  for (Int_t iComp = 3; iComp < 6; iComp++)
    covSeed(iComp, iComp) = covSeed(iComp - 3, iComp - 3);

  auto momReco = recoTrack -> GetMomentum();

  trackCand.setCovSeed(covSeed);
  trackCand.setPosMomSeed(posSeed, momReco, recoTrack -> GetCharge());

  genfit::Track *gfTrack = new ((*fGenfitTrackArray)[fGenfitTrackArray -> GetEntriesFast()]) genfit::Track(trackCand, *fMeasurementFactory);
  gfTrack -> addTrackRep(new genfit::RKTrackRep(STPID::GetPDG(recoTrack->GetPID())));

  fCurrentTrackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
  try {
    fKalmanFitter -> processTrackWithRep(gfTrack, fCurrentTrackRep, false);
  } catch (genfit::Exception &e) {}

  try {
    fCurrentFitStatus = gfTrack -> getFitStatus(fCurrentTrackRep);
    fCurrentFitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {
    return nullptr;
  }

  if (fCurrentFitStatus -> isFitted() == false || fCurrentFitStatus -> isFitConverged() == false)
    return nullptr;

  return gfTrack;
}

bool STGenfitTest2::ExtrapolateTrack(Double_t distance, TVector3 &position, Int_t direction)
{
  Double_t d = direction * 0.1 * distance;

  try {
    fCurrentTrackRep -> extrapolateBy(fCurrentFitState, d);
  } catch (genfit::Exception &e) {
    return false;
  }

  position = 10 * fCurrentFitState.getPos();
  return true;
}

bool STGenfitTest2::ExtrapolateTo(TVector3 to, TVector3 &position)
{
  try {
    fCurrentTrackRep -> extrapolateToPoint(fCurrentFitState, .1*to);
  } catch (genfit::Exception &e) {
    return false;
  }

  position = 10 * fCurrentFitState.getPos();
  return true;
}

void STGenfitTest2::GetTrackParameters(genfit::Track *gfTrack, TVector3 &mom, TVector3 &momentumTargetPlane, TVector3 &posTargetPlane)
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

  TVector3 posReco(-999,-999,-999);
  TMatrixDSym covMat(6,6);

  try { 
    trackRep -> extrapolateToPlane(fitState, fTargetPlane); 
    momentumTargetPlane = fitState.getMom();
    posTargetPlane = fitState.getPos();
  } catch (genfit::Exception &e) {
  }

  momentumTargetPlane = 1000*momentumTargetPlane;
  posTargetPlane = 10*posTargetPlane;

  try {
    fitState.getPosMomCov(posReco, mom, covMat);
  } catch (genfit::Exception &e) {
    return;
  }

  if (mom.Z() < 0)
    mom = -mom;
  mom = 1000*mom;
}

void STGenfitTest2::GetPosOnPlanes(genfit::Track *gfTrack, TVector3 &kyotoL, TVector3 &kyotoR, TVector3 &katana, TVector3 &neuland)
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

  try { 
    trackRep -> extrapolateToPlane(fitState, fKyotoLPlane); 
    kyotoL = 10*fitState.getPos();
  } catch (genfit::Exception &e) {
  }

  try { 
    trackRep -> extrapolateToPlane(fitState, fKyotoRPlane); 
    kyotoR = 10*fitState.getPos();
  } catch (genfit::Exception &e) {
  }

  try { 
    trackRep -> extrapolateToPlane(fitState, fKatanaPlane); 
    katana = 10*fitState.getPos();
  } catch (genfit::Exception &e) {
  }

  try { 
    trackRep -> extrapolateToPlane(fitState, fNeulandPlane); 
    neuland = 10*fitState.getPos();
  } catch (genfit::Exception &e) {
  }
}

void STGenfitTest2::GetMomentumWithVertex(genfit::Track *gfTrack, TVector3 posVertex, TVector3 &momVertex, TVector3 &pocaVertex)
{
  genfit::RKTrackRep *trackRep;
  genfit::MeasuredStateOnPlane fitState;
  genfit::FitStatus *fitStatus;

  try {
    trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
    fitState = gfTrack -> getFittedState();
    fitStatus = gfTrack -> getFitStatus(trackRep);
  } catch (genfit::Exception &e) {
    momVertex = TVector3(0,0,0);
  }

  try {
    trackRep -> extrapolateToPoint(fitState, .1*posVertex);
  } catch (genfit::Exception &e) {
  }

  TMatrixDSym covMat(6,6);

  try {
    fitState.getPosMomCov(pocaVertex, momVertex, covMat);
  } catch (genfit::Exception &e) {
    momVertex = TVector3(0,0,0);
  }

  momVertex = 1000*momVertex;
  pocaVertex = 10*pocaVertex;
}

Int_t
STGenfitTest2::DetermineCharge(STRecoTrack *recoTrack, TVector3 posVertex, Double_t &effCurvature1, Double_t &effCurvature2, Double_t &effCurvature3)
{
  effCurvature1 = 0.;
  effCurvature2 = 0.;
  effCurvature3 = 0.;

  auto helixTrack = recoTrack -> GetHelixTrack();
  auto gfTrack = recoTrack -> GetGenfitTrack();

  genfit::RKTrackRep *trackRep;
  genfit::MeasuredStateOnPlane fitState;
  try {
    trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
    fitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {
    return false;
  }

  auto clusterArray = helixTrack -> GetClusterArray();
  auto numClusters = clusterArray -> size();
  auto dedxArray = recoTrack -> GetdEdxPointArray();
  dedxArray -> clear();
  vector<STdEdxPoint> dedxArrayTemp;

  //////////////////////////////////////////////////////////////////////////////////
  // Set dedx points
  //////////////////////////////////////////////////////////////////////////////////
  for (auto iCluster = 0; iCluster < numClusters; iCluster++)
  {
    auto cluster = clusterArray -> at(iCluster);
    if (!cluster -> IsStable())
      continue;

    STdEdxPoint dedx;
    dedx.fClusterID = cluster -> GetClusterID();
    dedx.fPosition = cluster -> GetPosition();

    auto row = cluster -> GetRow();
    auto layer = cluster -> GetLayer();
    auto position = cluster -> GetPosition();

    bool buildByLayer = true;
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
      //cluster -> SetIsStable(false); //TODO
      continue;
    }

    Double_t dE = cluster -> GetCharge();
    Double_t dx = cluster -> GetLength();

    if (dx > 20) {
      //cluster -> SetIsStable(false); //TODO
      continue;
    }

    dedx.fdE = dE;
    dedx.fdx = dx;
    dedx.fLength = 88;
    dedxArrayTemp.push_back(dedx);
  }

  auto numdedx = dedxArrayTemp.size();
  if (numdedx < 5)
    return 0;

  //////////////////////////////////////////////////////////////////////////////////
  // Length Calculation
  //////////////////////////////////////////////////////////////////////////////////
  bool failExtrapolation = false;
  for (auto idedx = 0; idedx < numdedx; ++idedx)
  {
    auto dedx = dedxArrayTemp.at(idedx);

    try { trackRep -> extrapolateToPoint(fitState, .1*posVertex); }
    catch (genfit::Exception &e) { failExtrapolation = true; break; }

    Double_t length = -999;
    try { length = std::abs(10*trackRep -> extrapolateToPoint(fitState, .1*dedx.fPosition)); }
    catch (genfit::Exception &e) { failExtrapolation = true; break; }
    dedx.fLength = length;

    dedxArray -> push_back(dedx);
  }

  if (failExtrapolation)
    return 0;

  sort(dedxArray->begin(), dedxArray->end(), STdEdxPointSortByLength());

  //////////////////////////////////////////////////////////////////////////////////
  // Effective Curvature
  //////////////////////////////////////////////////////////////////////////////////

  auto posdEdx0 = (dedxArray -> at(0)).fPosition;
  TVector3 pocaPoint0;
  try {
    trackRep -> extrapolateToPoint(fitState, .1*posdEdx0);
    pocaPoint0 = 10*fitState.getPos();
  } catch (genfit::Exception &e) {
    return 0;
  }

  auto posdEdx1 = (dedxArray -> at(1)).fPosition;
  TVector3 pocaPointB;
  try {
    trackRep -> extrapolateToPoint(fitState, .1*posdEdx1);
    pocaPointB = 10*fitState.getPos();
  } catch (genfit::Exception &e) {
    return 0;
  }
  TVector3 pocaPointC = pocaPointB;

  // 1
  TVector3 point0ToB = pocaPointB - pocaPoint0;
  TVector3 point0ToC = point0ToB;
  // 2
  TVector3 pointAToB = pocaPointB - pocaPoint0;
  TVector3 pointBToC = pointAToB;
  // 3
  auto posdEdxEnd = (dedxArray -> at(numdedx-1)).fPosition;
  TVector3 pocaPointEnd;
  try {
    trackRep -> extrapolateToPoint(fitState, .1*posdEdxEnd);
    pocaPointEnd = 10*fitState.getPos();
  } catch (genfit::Exception &e) {
    return 0;
  }
  STGeoLine line0ToEnd(pocaPoint0, pocaPointEnd);
  auto lineToPoint = line0ToEnd.POCATo(pocaPointB);
  effCurvature3 += ((line0ToEnd.Direction()).Cross(lineToPoint)).Y()/2.;

  for (auto idedx = 2; idedx < numdedx; ++idedx)
  {
    point0ToB = point0ToC;

    pocaPointB = pocaPointC;
    pointAToB = pointBToC;

    auto dedx = dedxArray -> at(idedx);
    auto posdEdx = dedx.fPosition;

    try {
      trackRep -> extrapolateToPoint(fitState, .1*posdEdx);
      pocaPointC = 10*fitState.getPos();
    } catch (genfit::Exception &e) {
      failExtrapolation = true;
      break;
    }

    point0ToC = pocaPointC - pocaPoint0;
    point0ToB.SetY(0);
    point0ToC.SetY(0);
    effCurvature1 += ((point0ToB).Cross(point0ToC)).Y()/2.;

    pointBToC = pocaPointC - pocaPointB;
    pointAToB.SetY(0);
    pointBToC.SetY(0);
    effCurvature2 += ((pointAToB).Cross(pointBToC)).Y()/2.;

    lineToPoint = line0ToEnd.POCATo(pocaPointC);
    effCurvature3 += ((line0ToEnd.Direction()).Cross(lineToPoint)).Y()/2.;
  }

  if (failExtrapolation)
    return 0;

  Int_t charge = effCurvature1 > 0 ? -1 : 1;

  return charge;
}

bool 
STGenfitTest2::GetdEdxPointsByLength(genfit::Track *gfTrack, STHelixTrack *helixTrack, vector<STdEdxPoint> *dEdxPointArray)
{
  Int_t numPoints = helixTrack -> GetNumStableClusters(); if (numPoints < 3)
    return false;

  genfit::RKTrackRep *trackRep;
  genfit::MeasuredStateOnPlane fitState;
  try {
    trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
    fitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {
    return false;
  }

  auto clusterArray = helixTrack -> GetClusterArray();
  auto numClusters = clusterArray -> size();

  STHitCluster *preCluster = clusterArray -> at(0);
  TVector3 position = preCluster -> GetPosition();

  Double_t dLength;
  try {
    dLength = trackRep -> extrapolateToPoint(fitState, .1*position);
  } catch (genfit::Exception &e) {
    return false;
  }
  dLength = 0;

  for (auto iCluster = 1; iCluster < numClusters; iCluster++)
  {
    auto curCluster = clusterArray -> at(iCluster);

    Double_t length = .5*std::abs(dLength);

    position = curCluster -> GetPosition();
    try {
      dLength = trackRep -> extrapolateToPoint(fitState, .1*position);
      curCluster -> SetPOCA(10*fitState.getPos());
    } catch (genfit::Exception &e) {
      dEdxPointArray -> clear();
      return false;
    }

    length += .5*std::abs(dLength);
    preCluster -> SetLength(10*length);

    if (preCluster -> IsStable())
    {
      Double_t dE = preCluster -> GetCharge();
      Double_t dx = preCluster -> GetLength();
      dEdxPointArray -> push_back(STdEdxPoint(-1, dE, dx, -999));
    }

    preCluster = curCluster;
  }

  return true;
}

bool 
STGenfitTest2::GetdEdxPointsByLayerRow(genfit::Track *gfTrack, STHelixTrack *helixTrack, vector<STdEdxPoint> *dEdxPointArray)
{
  Int_t numPoints = helixTrack -> GetNumStableClusters();
  if (numPoints < 3)
    return false;

  genfit::RKTrackRep *trackRep;
  genfit::MeasuredStateOnPlane fitState;
  try {
    trackRep = (genfit::RKTrackRep *) gfTrack -> getTrackRep(0);
    fitState = gfTrack -> getFittedState();
  } catch (genfit::Exception &e) {
    return false;
  }

  auto clusterArray = helixTrack -> GetClusterArray();
  auto numClusters = clusterArray -> size();

  for (auto iCluster = 0; iCluster < numClusters; iCluster++)
  {
    auto cluster = clusterArray -> at(iCluster);
    if (!cluster -> IsStable())
      continue;

    auto row = cluster -> GetRow();
    auto layer = cluster -> GetLayer();
    auto position = cluster -> GetPosition();

    bool buildByLayer = true;
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
      //cluster -> SetIsStable(false); //TODO
      continue;
    }

    Double_t dE = cluster -> GetCharge();
    Double_t dx = cluster -> GetLength();

    if (dx > 20) {
      //cluster -> SetIsStable(false); //TODO
      continue;
    }

    dEdxPointArray -> push_back(STdEdxPoint(cluster -> GetClusterID(), dE, dx, -999));
  }

  return true;
}
