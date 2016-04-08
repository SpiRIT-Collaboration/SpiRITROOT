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
#include "STRiemannHit.hh"
#include "STTrack.hh"
#include "STTrackCandidate.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"
#include "STDatabasePDG.hh"

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
#include "Math/DistFunc.h"

//#define DEBUG

ClassImp(STGenfitTask);

STGenfitTask::STGenfitTask()
{
  fIsPersistence = kFALSE;

  fLogger = FairLogger::GetLogger();
  fPar = NULL;

//  fTrackArray = new TClonesArray("genfit::Track");
//  fTrackArray -> SetOwner();
  fTrackArray = new TClonesArray("STTrack");
  fVertexArray = new TClonesArray("genfit::GFRaveVertex");
  fRiemannTrackArray = NULL;
  fHitClusterArray = new TClonesArray("STHitCluster");
  fEventArray = new TClonesArray("STEvent");

  fTPCDetID = 0;

  fMinIterations = 5;
  fMaxIterations = 20;
  fFitter = NULL;

  fIsFindVertex = kFALSE;
  fVertexFactory = NULL;

  fIsDisplay = kFALSE;
  fDisplay = NULL;
  
  fPDGCandidates = STDatabasePDG::Instance() -> GetPDGCandidateArray();
}

STGenfitTask::~STGenfitTask()
{
}

void STGenfitTask::SetPersistence(Bool_t value)     { fIsPersistence = value; }
void STGenfitTask::SetMaxIterations(Int_t value)    { fMaxIterations = value; }
void STGenfitTask::SetFindVertex(Bool_t value)      { fIsFindVertex = value; }
void STGenfitTask::SetDisplay(Bool_t value)         { fIsDisplay = value; }

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
  //  fFitter = new genfit::KalmanFitterRefTrack();
  fFitter = new genfit::DAF();
  fFitter -> setMinIterations(fMinIterations);
  fFitter -> setMaxIterations(fMaxIterations);

  if (fIsFindVertex == kTRUE) {
    fVertexFactory = new genfit::GFRaveVertexFactory(/* verbosity */ 2, /* useVacuumPropagator */ kFALSE);
    //fVertexFactory -> setMethod("kalman-smoothing:1");
    fVertexFactory -> setMethod("avf-smoothing:1-Tini:256-ratio:0.25-sigmacut:5");

    ioMan -> Register("STVertex", "SPiRIT", fVertexArray, fIsPersistence);
  }

  fMeasurementProducer = new genfit::MeasurementProducer<STHitCluster, genfit::STSpacepointMeasurement>(fHitClusterArray);
  fMeasurementFactory = new genfit::MeasurementFactory<genfit::AbsMeasurement>();
  fMeasurementFactory -> addProducer(fTPCDetID, fMeasurementProducer); // detector ID of TPC is 0

  genfit::FieldManager::getInstance() -> init(new genfit::ConstField(0., 5., 0.)); // 0.5 T = 5 kGauss
  genfit::MaterialEffects *materialEffects = genfit::MaterialEffects::getInstance();
  materialEffects -> init(new genfit::TGeoMaterialInterface());
  //materialEffects -> setNoiseCoulomb(kFALSE);
  //materialEffects -> setNoiseBrems(kFALSE);
  //materialEffects -> setNoiseBetheBloch(kFALSE);

  if (fIsDisplay)
    fDisplay = genfit::EventDisplay::getInstance();

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

  fVertexArray -> Delete();

  vector<genfit::Track *> tracks;
  TVector3 posSeed;
  TMatrixDSym covSeed(6);

  STEvent *event = (STEvent *) fEventArray -> At(0);

  Int_t numTrackCand = fRiemannTrackArray -> GetEntriesFast();
  Int_t numTrack = 0;
  for (Int_t iTrackCand = 0; iTrackCand < numTrackCand; iTrackCand++) {

#ifdef DEBUG
    fLogger -> Info(MESSAGE_ORIGIN, Form("Track Candidate: %d/%d!", iTrackCand, numTrackCand));
#endif
    STRiemannTrack *riemannTrack = (STRiemannTrack *) fRiemannTrackArray -> At(iTrackCand);
    if (!(riemannTrack -> IsFitted())) continue;

    fHitClusterArray -> Delete();

    genfit::TrackCand gfTrackCand;

    UInt_t numHits = riemannTrack -> GetNumHits();

    // First hit position is used as starting position of the initial track
    STRiemannHit *hit = riemannTrack -> GetFirstHit();
    STHitCluster *cluster = event -> GetCluster(hit -> GetHit() -> GetClusterID());
    posSeed = cluster -> GetPosition();
    posSeed.SetMag(posSeed.Mag()/10.);

    // First hit covariance matrix is used as covariance matrix seed of the initial track 
    TMatrixD covMatrix = cluster -> GetCovMatrix();
    //    covMatrix.Print();

    Int_t trackID = fTrackArray -> GetEntriesFast();
    STTrack *recoTrack = (STTrack *) fTrackArray -> ConstructedAt(trackID);
    recoTrack -> SetTrackID(trackID);
    recoTrack -> SetRiemannID(iTrackCand);

    std::vector<STHitCluster *> clusters;
    for (UInt_t iHit = 0; iHit < numHits; iHit++) {
      hit = riemannTrack -> GetHit(iHit);
      Int_t id = hit -> GetHit() -> GetClusterID();
      cluster = event -> GetCluster(id);

      recoTrack -> AddHitID(id);
      clusters.push_back(cluster);
    }

    std::sort(clusters.begin(), clusters.end(), [](const STHitCluster *a, const STHitCluster *b) { return a -> GetPosition().Z() < b -> GetPosition().Z(); });

    for (UInt_t iHit = 0; iHit < numHits; iHit++) {
      cluster = clusters.at(iHit);

      new ((*fHitClusterArray)[iHit]) STHitCluster(*cluster);
      gfTrackCand.addHit(fTPCDetID, iHit);
    }

    Double_t dip = riemannTrack -> GetDip();
    Double_t momSeedMag = riemannTrack -> GetMom(5.);
    TVector3 momSeed(0., 0., momSeedMag);
    momSeed.SetTheta(TMath::Pi()/2. - dip);

    //////////////////////////////////////////////////////
    // 6Dsym covariant matrix for genfit TrackCand  <---- cross check if it is initial value dependent
    //////////////////////////////////////////////////////    

    for (Int_t iComp = 0; iComp < 3; iComp++)
      //      covSeed(iComp, iComp) = 0.5*0.5;
      covSeed(iComp, iComp) = covMatrix(iComp, iComp)/100.;
    for (Int_t iComp = 3; iComp < 6; iComp++)
      //      covSeed(iComp, iComp) = 0.5*0.5;
      covSeed(iComp, iComp) = covSeed(iComp - 3, iComp - 3);

    gfTrackCand.setCovSeed(covSeed);
    //gfTrackCand.setPosMomSeedAndPdgCode(posSeed, momSeed, 2212);
    gfTrackCand.setPosMomSeed(posSeed, momSeed, 1);

    Double_t totalLength = 0;
    Int_t totalEloss = 0;
    Int_t totaldEdx = 0;

    Bool_t getdedx = GetdEdxFromRiemann(event, riemannTrack, totalLength, totalEloss);

    if (getdedx == kFALSE || totalLength < 0) {
      totalLength = -1;
      totaldEdx = -1;
    }
    else
      totaldEdx = totalEloss/totalLength;

    recoTrack -> SetTrackLength(totalLength);
    recoTrack -> SetTotaldEdx(totaldEdx);

    try {
      genfit::Track *gfTrackFit = new genfit::Track(gfTrackCand, *fMeasurementFactory);

      genfit::RKTrackRep *trackRep;
      for (auto pdgCand : *fPDGCandidates) 
      {
        trackRep = new genfit::RKTrackRep(pdgCand);
        gfTrackFit -> addTrackRep(trackRep);
      }
      fFitter -> processTrack(gfTrackFit);

      gfTrackFit -> determineCardinalRep();
      trackRep = (genfit::RKTrackRep *) gfTrackFit -> getCardinalRep();

      Bool_t isFit, isFitFull, isFitPart, hasChanged, isPruned;
      Int_t nfail;
      Double_t pval;

      /*
      isFit = gfTrackFit -> getFitStatus(trackRep)->isFitted();
      isFitFull = gfTrackFit -> getFitStatus(trackRep)->isFitConvergedFully();
      isFitPart = gfTrackFit -> getFitStatus(trackRep)->isFitConvergedPartially();
      nfail = gfTrackFit -> getFitStatus(trackRep)->getNFailedPoints();
      pval = gfTrackFit -> getFitStatus(trackRep)->getPVal();
      hasChanged = gfTrackFit -> getFitStatus(trackRep)->hasTrackChanged();
      isPruned = gfTrackFit -> getFitStatus(trackRep)->isTrackPruned();

      std::cout << "Is Fitted:\t" << isFit << std::endl;
      std::cout << "Is FitConvergedFully:\t" << isFitFull << std::endl;
      std::cout << "Is FitConvergedPartially:\t" << isFitPart << std::endl;
      std::cout << "getNFailedPoints:\t" << nfail << std::endl;
      std::cout << "pval:\t" << pval << std::endl;
      std::cout << "hasTrackChanged:\t" << hasChanged << std::endl;
      std::cout << "isTrackPruned:\t" << isPruned << std::endl;
      */

      assert(gfTrackFit -> checkConsistency());

      TVector3 recopos(0,0,0);
      TVector3 recop(0,0,0);
      TMatrixDSym covv(6,6);
      Int_t charge;
      Double_t prob;
      Double_t probmin = -1.;
      Int_t pdgId;

      Double_t bChi2, fChi2, bNdf, fNdf;
      fFitter -> getChiSquNdf(gfTrackFit, gfTrackFit -> getCardinalRep(), bChi2, fChi2, bNdf, fNdf);

      gfTrackFit -> getFittedState().getPosMomCov(recopos, recop, covv);
      charge = gfTrackFit -> getFittedState().getCharge();

      /*
      //pdgId = gfTrackFit -> getFittedState().getPDG();
      std::cout << "##################################################" << std::endl;
      std::cout << "Riemann: " << riemannTrack -> GetMom(5.)*1000 << " MeV/c  " << std::endl;
      std::cout << "Genfit total momentum: " << recop.Mag()*1000 << " MeV/c" << std::endl;
      std::cout << std::endl << "Reconstructed position: (" << recopos.X() << "," << recopos.Y() << "," << recopos.Z() << ")" << std::endl;
      std::cout << "Reconstructed momentum: (" << recop.X() << "," << recop.Y() << "," << recop.Z() << ")" << std::endl;
      std::cout << "Particle charge: " << charge << std::endl;
      std::cout << "##################################################" << std::endl;
       */

      //////////////////////////////////////
      // beam profile extrapolation
      //////////////////////////////////////
      TVector3 target(0, -21.33, -0.89);
      TVector3 ntarget(0, 0, 1);

      genfit::StateOnPlane state = gfTrackFit -> getFittedState();
      genfit::SharedPlanePtr plane;
      plane = genfit::SharedPlanePtr(new genfit::DetPlane(target, ntarget));
      trackRep -> extrapolateToPlane(state, plane); 
      TVector3 beampos = state.getPos();
      TVector3 beammom = state.getMom();

      ////////////////////////////////////////
      // kyoto left extrapolation
      ////////////////////////////////////////
      /*
      TVector3 paddleL(75.8, -21.33, 84.5);
      TVector3 npaddleL(-1, 0, 0);
      genfit::StateOnPlane stateL = gfTrackFit -> getFittedState();
      genfit::SharedPlanePtr planeL;
      planeL = genfit::SharedPlanePtr(new genfit::DetPlane(paddleL, npaddleL));
      trackRep -> extrapolateToPlane(stateL, planeL); 
      TVector3 KyotoLpos = stateL.getPos();

      ////////////////////////////////////////
      // kyoto right extrapolation
      ////////////////////////////////////////
      TVector3 paddleR(-75.8, -21.33, 84.5);
      TVector3 npaddleR(1, 0, 0);
      genfit::StateOnPlane stateR = gfTrackFit -> getFittedState();
      genfit::SharedPlanePtr planeR;
      planeR = genfit::SharedPlanePtr(new genfit::DetPlane(paddleR, npaddleR));
      trackRep -> extrapolateToPlane(stateR, planeR); 
      TVector3 KyotoRpos = stateR.getPos();
      */

      ////////////////////////////////////////
      // katana extrapolation
      ////////////////////////////////////////
      TVector3 paddleK(-20, -21.33, 186.7);
      TVector3 npaddleK(0, 0, -1);
      genfit::StateOnPlane stateK = gfTrackFit -> getFittedState();
      genfit::SharedPlanePtr planeK;
      planeK = genfit::SharedPlanePtr(new genfit::DetPlane(paddleK, npaddleK));
      trackRep -> extrapolateToPlane(stateK, planeK, true); 
      TVector3 Katanapos = stateK.getPos();

      trackRep -> extrapolateToPlane(state, plane); 

      Double_t probability = gfTrackFit -> getFitStatus(trackRep) -> getPVal();

      //if (probability > 0)
      {
        STTrackCandidate *recoTrackCand = new STTrackCandidate();
        recoTrackCand -> SetVertex(recopos*10.);
        recoTrackCand -> SetBeamVertex(beampos*10.);
        recoTrackCand -> SetBeamMomentum(beammom);
        //	recoTrackCand -> SetKyotoLHit(KyotoLpos*10.);
        //	recoTrackCand -> SetKyotoRHit(KyotoRpos*10.);
        recoTrackCand -> SetKatanaHit(Katanapos*10.);
        recoTrackCand -> SetMomentum(recop*1000.);
        recoTrackCand -> SetPID(gfTrackFit -> getFittedState().getPDG());
        recoTrackCand -> SetMass(gfTrackFit -> getFittedState().getMass()*1000);
        recoTrackCand -> SetCharge(gfTrackFit -> getFittedState().getCharge());
        recoTrackCand -> SetProbability(probability);
        recoTrackCand -> SetChi2(fChi2);
        recoTrackCand -> SetNDF(fNdf);
        //recoTrackCand -> SetTrackLength(gfTrackFit -> getTrackLen());
        recoTrackCand -> SetTrackLength(totalLength); // TODO
        recoTrackCand -> SetTotaldEdx(totaldEdx); // TODO

        recoTrack -> AddTrackCandidate(recoTrackCand);
        recoTrack -> SelectTrackCandidate(recoTrackCand);

        // new ((*fTrackArray)[fTrackArray -> GetEntriesFast()]) genfit::Track(*gfTrackFit);
      }

      tracks.push_back(gfTrackFit);
      recoTrack -> SetIsFitted();

    } catch (genfit::Exception &e) {
      //std::cerr << e.what();
      //std::cerr << "Exception, next track" << std::endl;
    }
    recoTrack -> SetNDF(numHits);
  }

  vector<genfit::GFRaveVertex *> vertices;
  if (fIsFindVertex == kTRUE) {
    fVertexFactory -> findVertices(&vertices, tracks);

    for (UInt_t iVert = 0; iVert < vertices.size(); iVert++) {
      genfit::GFRaveVertex* vtx = static_cast<genfit::GFRaveVertex*>(vertices[iVert]);
      vertices[iVert] -> getPos().Print();

      new ((*fVertexArray)[iVert]) genfit::GFRaveVertex(*vtx);

      event -> AddVertex(vtx);
    }
  }

  if (fIsDisplay)
    fDisplay -> addEvent(tracks);
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStop("GenfitTask");
#endif
}

void
STGenfitTask::OpenDisplay() {
  if (fIsDisplay)
    fDisplay -> open();
}

Bool_t
STGenfitTask::GetdEdxFromRiemann(STEvent *event, STRiemannTrack *track, Double_t &totalLength, Int_t &totalEloss)
{
  const std::vector<STRiemannHit *> *hitArray = track -> GetHits();

  totalLength = 0; // total length of track
  totalEloss = 0; // total charge of track

  Int_t numHits = hitArray -> size();
  if(numHits <= 3) 
    return kFALSE; 

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

  return kTRUE;
}
