#include "STGenfitSinglePIDTask.hh"
#include "STDatabasePDG.hh"
#include "STTrack.hh"
#include "STVertex.hh"

#include "FairRun.h"
#include "FairRuntimeDb.h"
#include "FairRootManager.h"

#include "Track.h"

#include <iostream>
using namespace std;

ClassImp(STGenfitSinglePIDTask)

STGenfitSinglePIDTask::STGenfitSinglePIDTask()
: STRecoTask("GENFIT Single PID Task", 1, false)
{
}

STGenfitSinglePIDTask::STGenfitSinglePIDTask(Bool_t persistence)
: STRecoTask("GENFIT Single PID Task", 1, persistence)
{
}

STGenfitSinglePIDTask::~STGenfitSinglePIDTask()
{
}

void STGenfitSinglePIDTask::SetIterationCut(Int_t min, Int_t max) 
{
  fMinIterations = min;
  fMaxIterations = max;
}

void STGenfitSinglePIDTask::SetClusteringType(Int_t type) { fClusteringType = type; }

InitStatus
STGenfitSinglePIDTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fTrackArray = (TClonesArray *) fRootManager -> GetObject("STTrack");

  fGenfitTest = new STGenfitTestE();
  fGenfitTest -> SetMinIterations(fMinIterations);
  fGenfitTest -> SetMaxIterations(fMaxIterations);

  fVertexArray = new TClonesArray("STVertex");
  fRootManager -> Register("STVertex", "SpiRIT", fVertexArray, fIsPersistence);

  fVertexFactory = new genfit::GFRaveVertexFactory(/* verbosity */ 2, /* useVacuumPropagator */ kFALSE);
  //fVertexFactory -> setMethod("kalman-smoothing:1");
  fGFRaveVertexMethod = "avf-smoothing:1-Tini:256-ratio:0.25-sigmacut:5";
  fVertexFactory -> setMethod(fGFRaveVertexMethod.Data());

  if (fRecoHeader != nullptr) {
    fRecoHeader -> SetPar("genfit_minimumIteration", fMinIterations);
    fRecoHeader -> SetPar("genfit_maximumIteration", fMaxIterations);
    fRecoHeader -> SetPar("vertex_method", fGFRaveVertexMethod);
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }

  return kSUCCESS;
}

void STGenfitSinglePIDTask::Exec(Option_t *opt)
{
  fVertexArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  Int_t numRecoTracks = fTrackArray -> GetEntriesFast();

  vector<genfit::Track *> genfitTrackArray;
  for (auto iTrack = 0; iTrack < numRecoTracks; ++iTrack) {
    auto recoTrack = (STTrack *) fTrackArray -> At(iTrack);
    if (!recoTrack -> IsFitted())
      continue;
    genfitTrackArray.push_back(recoTrack -> GetGenfitTrack());
  }

  vector<genfit::GFRaveVertex *> vertices;
  try {
    fVertexFactory -> findVertices(&vertices, genfitTrackArray);
  } catch (...) {
  }

  Int_t numVertices = vertices.size();

  for (UInt_t iVert = 0; iVert < numVertices; iVert++) {
    genfit::GFRaveVertex* vertex = static_cast<genfit::GFRaveVertex*>(vertices[iVert]);

    Int_t numTracks = vertex -> getNTracks();
    for (Int_t iTrack = 0; iTrack < numTracks; iTrack++) {
      genfit::GFRaveTrackParameters *par = vertex -> getParameters(iTrack);
      const genfit::Track *track = par -> getTrack();

      std::vector<genfit::Track *>::iterator iter = std::find(genfitTrackArray.begin(), genfitTrackArray.end(), track);
      if (iter != genfitTrackArray.end()) {
        Int_t index = std::distance(genfitTrackArray.begin(), iter);
        STTrack *recoTrack = (STTrack *) fTrackArray -> At(index);
        genfit::Track *gfTrack = genfitTrackArray.at(index);
        if (recoTrack -> GetParentID() < 0) {
          recoTrack -> SetParentID(iVert);
          fGenfitTest -> SetTrackParameters(recoTrack, gfTrack, 10*vertex -> getPos());
        }
      }
    }

    new ((*fVertexArray)[iVert]) STVertex(*vertex);
    delete vertex;
  }
}
