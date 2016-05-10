#include "STGenfitETask.hh"
#include "STDatabasePDG.hh"
#include "STTrack.hh"

#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"

#include "Track.h"

#include <iostream>
using namespace std;

ClassImp(STGenfitETask)

STGenfitETask::STGenfitETask()
: STRecoTask("GENFIT Task", 1, false)
{
}

STGenfitETask::STGenfitETask(Bool_t persistence, Bool_t removeNoVertexEvent)
: STRecoTask("GENFIT Task", 1, persistence)
{
  fRemoveNoVertexEvent = removeNoVertexEvent;
}

STGenfitETask::~STGenfitETask()
{
}

void STGenfitETask::SetIterationCut(Int_t min, Int_t max) 
{
  fGenfitTest -> SetMinIterations(min);
  fGenfitTest -> SetMaxIterations(max);
}

InitStatus
STGenfitETask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fRiemannTrackArray = (TClonesArray *) fRootManager -> GetObject("STRiemannTrack");
  if (fRiemannTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find STRiemannTrack array!" << FairLogger::endl;
    return kERROR;
  }

  fHitClusterArray = (TClonesArray *) fRootManager -> GetObject("STHitCluster");
  if (fHitClusterArray == nullptr) {
    LOG(ERROR) << "Cannot find STHitCluster array!" << FairLogger::endl;
    return kERROR;
  }

  fTrackArray = new TClonesArray("STTrack");
  fRootManager -> Register("STTrack", "SpiRIT", fTrackArray, fIsPersistence);

  fGenfitTest = new STGenfitTestE();

  fVertexArray = new TClonesArray("genfit::GFRaveVertex");
  fRootManager -> Register("STVertex", "SpiRIT", fVertexArray, fIsPersistence);

  fVertexFactory = new genfit::GFRaveVertexFactory(/* verbosity */ 2, /* useVacuumPropagator */ kFALSE);
  //fVertexFactory -> setMethod("kalman-smoothing:1");
  fVertexFactory -> setMethod("avf-smoothing:1-Tini:256-ratio:0.25-sigmacut:5");

  return kSUCCESS;
}

void STGenfitETask::Exec(Option_t *opt)
{
  fTrackArray -> Clear("C");
  fVertexArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  vector<genfit::Track *> genfitTrackArray;

  fGenfitTest -> Init();

  Int_t numTrackCand = fRiemannTrackArray -> GetEntriesFast();
  for (Int_t iRiemann = 0; iRiemann < numTrackCand; iRiemann++) 
  {
    STRiemannTrack *riemannTrack = (STRiemannTrack *) fRiemannTrackArray -> At(iRiemann);
    if (!(riemannTrack -> IsFitted())) 
      continue;

    Int_t trackID = fTrackArray -> GetEntriesFast();
    STTrack *recoTrack = (STTrack *) fTrackArray -> ConstructedAt(trackID);
    recoTrack -> SetTrackID(trackID);
    recoTrack -> SetRiemannID(iRiemann);

    genfit::Track *track = fGenfitTest -> FitTrack(recoTrack, fHitClusterArray, riemannTrack);
    if (track != nullptr)
    {
      fGenfitTest -> SetTrack(recoTrack, track);
      if (recoTrack -> IsFitted() == kFALSE) {
        fTrackArray -> Remove(recoTrack);
        continue;
      }
      genfitTrackArray.push_back(track);
    }
    else
      fTrackArray -> Remove(recoTrack);
  }
  fTrackArray -> Compress();

  vector<genfit::GFRaveVertex *> vertices;
  fVertexFactory -> findVertices(&vertices, genfitTrackArray);

  Int_t numVertices = vertices.size();

  if (numVertices == 0) {
    fTrackArray -> Clear("C");
    return;
  }

  for (UInt_t iVert = 0; iVert < numVertices; iVert++) {
    genfit::GFRaveVertex* vertex = static_cast<genfit::GFRaveVertex*>(vertices[iVert]);

    Int_t numTracks = vertex -> getNTracks();
    for (Int_t iTrack = 0; iTrack < numTracks; iTrack++) {
      genfit::GFRaveTrackParameters *par = vertex -> getParameters(iTrack);
      const genfit::Track *track = par -> getTrack();

      std::vector<genfit::Track *>::iterator iter = std::find(genfitTrackArray.begin(), genfitTrackArray.end(), track);
      if (iter != genfitTrackArray.end())
      {
        Int_t index = std::distance(genfitTrackArray.begin(), iter);
        genfit::Track *gfTrack = genfitTrackArray.at(iTrack);
        STTrack *recoTrack = (STTrack *) fTrackArray -> At(index);
        if (recoTrack -> GetParentID() < 0)
        {
          recoTrack -> SetParentID(iVert);
        }
      }
    }
    new ((*fVertexArray)[iVert]) genfit::GFRaveVertex(*vertex);
    delete vertex;
  }

  if (fRemoveNoVertexEvent)
  {
    Int_t numRecoTracks = fTrackArray -> GetEntriesFast();
    for (Int_t iReco = 0; iReco < numRecoTracks; iReco++) {
      STTrack *recoTrack = (STTrack *) fTrackArray -> At(iReco);
      if (recoTrack -> GetParentID() < 0)
        fTrackArray -> Remove(recoTrack);
    }
    fTrackArray -> Compress();
  }
}

void STGenfitETask::SetRemoveNoVertexEvent(Bool_t val) { fRemoveNoVertexEvent = val; }
