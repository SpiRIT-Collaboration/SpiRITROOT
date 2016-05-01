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

ClassImp(STGenfitTask);

STGenfitTask::STGenfitTask()
{
  fIsPersistence = kFALSE;

  fLogger = FairLogger::GetLogger();
  fPar = NULL;

  fTrackArray = new TClonesArray("STTrack");
  fHitClusterArray = new TClonesArray("STHitCluster");
  fVertexArray = new TClonesArray("genfit::GFRaveVertex");

  fEventArray = NULL;
  fRiemannTrackArray = NULL;

  fIsFindVertex = kTRUE;
  fVertexFactory = NULL;

  fIsDisplay = kFALSE;

  fGenfitTest = new STGenfitTest();
}

STGenfitTask::STGenfitTask(Bool_t persistence)
:STGenfitTask()
{
  fIsPersistence = persistence;
}

STGenfitTask::~STGenfitTask()
{
}

void STGenfitTask::SetMinIterations(Int_t value) { 
  fGenfitTest -> SetMinIterations(value);
}
void STGenfitTask::SetMaxIterations(Int_t value) { 
  fGenfitTest -> SetMaxIterations(value);
}

void STGenfitTask::SetPersistence(Bool_t value)     { fIsPersistence = value; }
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

  if (fIsFindVertex == kTRUE) {
    fVertexFactory = new genfit::GFRaveVertexFactory(/* verbosity */ 2, /* useVacuumPropagator */ kFALSE);
    //fVertexFactory -> setMethod("kalman-smoothing:1");
    fVertexFactory -> setMethod("avf-smoothing:1-Tini:256-ratio:0.25-sigmacut:5");

    ioMan -> Register("STVertex", "SPiRIT", fVertexArray, fIsPersistence);
  }

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
  fTrackArray -> Clear("C");

  if (fRiemannTrackArray -> GetEntriesFast() == 0)
    return;

  fVertexArray -> Delete();

  vector<genfit::Track *> genfitTrackArray;

  STEvent *event = (STEvent *) fEventArray -> At(0);

  fGenfitTest -> Init();

  Int_t numTrackCand = fRiemannTrackArray -> GetEntriesFast();
  for (Int_t iRiemann = 0; iRiemann < numTrackCand; iRiemann++) 
  {
    STRiemannTrack *riemannTrack = (STRiemannTrack *) fRiemannTrackArray -> At(iRiemann);
    if (!(riemannTrack -> IsFitted())) continue;

    Int_t trackID = fTrackArray -> GetEntriesFast();
    STTrack *recoTrack = (STTrack *) fTrackArray -> ConstructedAt(trackID);
    recoTrack -> SetTrackID(trackID);
    recoTrack -> SetRiemannID(iRiemann);

    genfit::Track *track = fGenfitTest -> FitTrack(recoTrack, event, riemannTrack);
    if (track != nullptr)
      genfitTrackArray.push_back(track);
  }

  vector<genfit::GFRaveVertex *> vertices;
  if (fIsFindVertex == kTRUE) {
    fVertexFactory -> findVertices(&vertices, genfitTrackArray);

    for (UInt_t iVert = 0; iVert < vertices.size(); iVert++) {
      genfit::GFRaveVertex* vtx = static_cast<genfit::GFRaveVertex*>(vertices[iVert]);
      new ((*fVertexArray)[iVert]) genfit::GFRaveVertex(*vtx);
      event -> AddVertex(vtx);
      delete vtx;
    }
  }

#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStop("GenfitTask");
#endif
}

void STGenfitTask::SetTargetPlane(TVector3 position, TVector3 normal)
{
  fGenfitTest -> SetTargetPlane(position, normal);
}
