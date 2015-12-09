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

// SPiRITROOT classes
#include "STEvent.hh"
#include "STGenfitTask.hh"
#include "STRiemannTrack.hh"
#include "STRiemannHit.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

// GENFIT2 classes
#include "Track.h"
#include "TrackCand.h"
#include "RKTrackRep.h"
#include "Exception.h"

// STL
#include <iostream>

// ROOT classes
#include "TMatrixDSym.h"
#include "TMatrixD.h"
#include "TMath.h"
#include "TGeoManager.h"

//#define DEBUG

ClassImp(STGenfitTask);

STGenfitTask::STGenfitTask()
{
  fIsPersistence = kFALSE;

  fLogger = FairLogger::GetLogger();
  fPar = NULL;

  fTrackArray = new TClonesArray("genfit::Track");
  fVertexArray = new TClonesArray("genfit::GFRaveVertex");
  fRiemannTrackArray = NULL;
  fHitClusterArray = new TClonesArray("STHitCluster");
  fEventArray = new TClonesArray("STEvent");

  fTPCDetID = 0;

  fMaxIterations = 5;
  fFitter = NULL;

  fIsFindVertex = kFALSE;
  fVertexFactory = NULL;
}

STGenfitTask::~STGenfitTask()
{
}

void STGenfitTask::SetPersistence(Bool_t value)     { fIsPersistence = value; }
void STGenfitTask::SetMaxIterations(Int_t value)    { fMaxIterations = value; }
void STGenfitTask::SetFindVertex(Bool_t value)      { fIsFindVertex = value; }

InitStatus
STGenfitTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fRiemannTrackArray = (TClonesArray *) ioMan -> GetObject("STRiemannTrack");
  if (fRiemannTrackArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STRiemannTrack array!");
    return kERROR;
  }

  fEventArray = (TClonesArray *) ioMan -> GetObject("STEvent");
  if (fEventArray == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find STEvent array!");
    return kERROR;
  }

  ioMan -> Register("STTrack", "SPiRIT", fTrackArray, fIsPersistence);

  // GENFIT initialization
  fFitter = new genfit::KalmanFitterRefTrack();
  fFitter -> setMinIterations(fMaxIterations);

  if (fIsFindVertex == kTRUE) {
    fVertexFactory = new genfit::GFRaveVertexFactory(/* verbosity */ 2, /* useVacuumPropagator */ kFALSE);
    fVertexFactory -> setMethod("kalman-smoothing:1");

    ioMan -> Register("STVertex", "SPiRIT", fVertexArray, fIsPersistence);
  }

  fMeasurementProducer = new genfit::MeasurementProducer<STHitCluster, genfit::STSpacepointMeasurement>(fHitClusterArray);

  fMeasurementFactory = new genfit::MeasurementFactory<genfit::AbsMeasurement>();
  fMeasurementFactory -> addProducer(fTPCDetID, fMeasurementProducer); // detector ID of TPC is 0

  genfit::FieldManager::getInstance() -> init(new genfit::ConstField(0., 5., 0.)); // 0.5 T = 5 kGauss
  genfit::MaterialEffects::getInstance() -> init(new genfit::TGeoMaterialInterface());

  return kSUCCESS;
}

void
STGenfitTask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "STDigiPar not found!!");
}

void
STGenfitTask::Exec(Option_t *opt)
{
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStart("GenfitTask");
#endif
#ifdef DEBUG
  fLogger -> Info(MESSAGE_ORIGIN, "Start STGenfitTask!");
#endif
  fTrackArray -> Delete();

  if (fRiemannTrackArray -> GetEntriesFast() == 0)
    return;

  vector<genfit::Track *> tracks;
  TVector3 posSeed;
  TMatrixDSym covSeed(6);

  STEvent *event = (STEvent *) fEventArray -> At(0);

  Int_t numTrackCand = fRiemannTrackArray -> GetEntriesFast();
  for (Int_t iTrackCand = 0; iTrackCand < numTrackCand; iTrackCand++) {

#ifdef DEBUG
    fLogger -> Info(MESSAGE_ORIGIN, Form("Track Candidate: %d/%d!", iTrackCand, numTrackCand));
#endif

    STRiemannTrack *riemannTrack = (STRiemannTrack *) fRiemannTrackArray -> At(iTrackCand);
    if (!(riemannTrack -> IsFitted())) continue;

    fHitClusterArray -> Clear();
    genfit::TrackCand trackCand;

    UInt_t numHits = riemannTrack -> GetNumHits();

    // First hit position is used as starting position of the initial track
    STRiemannHit *hit = riemannTrack -> GetFirstHit();
    STHitCluster *cluster = event -> GetCluster(hit -> GetHit() -> GetClusterID());
    posSeed = cluster -> GetPosition();
    posSeed.SetMag(posSeed.Mag()/10.);

    // First hit covariance matrix is used as covariance matrix seed of the initial track
    TMatrixD covMatrix = cluster -> GetCovMatrix();
    for (Int_t iComp = 0; iComp < 3; iComp++)
      //covSeed(iComp, iComp) = covMatrix(iComp, iComp)/100.;
      covSeed(iComp, iComp) = 0.08*0.08;
    for (Int_t iComp = 3; iComp < 6; iComp++)
      covSeed(iComp, iComp) = covSeed(iComp - 3, iComp - 3)/(numHits*numHits)/3.;

    for (UInt_t iHit = 0; iHit < numHits; iHit++) {
      hit = riemannTrack -> GetHit(iHit);
      cluster = event -> GetCluster(hit -> GetHit() -> GetClusterID());

      new ((*fHitClusterArray)[iHit]) STHitCluster(*cluster);
      trackCand.addHit(fTPCDetID, iHit);
    }

    Double_t dip = riemannTrack -> GetDip();
    Double_t momSeedMag = riemannTrack -> GetMom(5.)*1000.;
    TVector3 momSeed(0., 0., momSeedMag);
    momSeed.SetTheta(TMath::Pi()/2. - dip);

    trackCand.setPosMomSeedAndPdgCode(posSeed, momSeed, 2212);
    trackCand.setCovSeed(covSeed);

    genfit::Track trackFit(trackCand, *fMeasurementFactory, new genfit::RKTrackRep(2212));
    try {
      fFitter -> processTrack(&trackFit);
    } catch (genfit::Exception &e) {
      std::cerr << e.what();
      std::cerr << "Exception, next track" << std::endl;
      continue;
    }

    tracks.push_back(&trackFit);
    new ((*fTrackArray)[iTrackCand]) genfit::Track(trackFit);
  }

  vector<genfit::GFRaveVertex *> vertices;
  if (fIsFindVertex == kTRUE) {
    fVertexFactory -> findVertices(&vertices, tracks);

    for (UInt_t iVert = 0; iVert < vertices.size(); iVert++) {
      genfit::GFRaveVertex* vtx = static_cast<genfit::GFRaveVertex*>(vertices[iVert]);

      new ((*fVertexArray)[iVert]) genfit::GFRaveVertex(*vtx);
    }
  }
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStop("GenfitTask");
#endif
}
