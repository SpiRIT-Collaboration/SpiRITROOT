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
void STGenfitSinglePIDTask::SetConstantField()            { fIsSamurai = kFALSE; }

InitStatus
STGenfitSinglePIDTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fHelixTrackArray = (TClonesArray *) fRootManager -> GetObject("STHelixTrack");
  if (fHelixTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find STHelixTrack array!" << FairLogger::endl;
    return kERROR;
  }

  fTrackPreArray = (TClonesArray *) fRootManager -> GetObject("STTrackPre");

  fTrackArray = new TClonesArray("STTrack");
  fRootManager -> Register("STTrack", "SpiRIT", fTrackArray, fIsPersistence);

  fTrackCandArray = new TClonesArray("STTrackCandidate");

  fGenfitTest = new STGenfitTestE(fIsSamurai);
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
  fTrackArray -> Clear("C");
  fTrackCandArray -> Clear("C");
  fVertexArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  vector<genfit::Track *> genfitTrackArray;

  fGenfitTest -> Init();

  Int_t numTrackCand = fHelixTrackArray -> GetEntriesFast();
  for (Int_t iHelixTrack = 0; iHelixTrack < numTrackCand; iHelixTrack++)
  {
    STHelixTrack *helixTrack = (STHelixTrack *) fHelixTrackArray -> At(iHelixTrack);
    auto genfitID = helixTrack -> GetGenfitID();
    if (genfitID < 0)
      continue;

    auto preTrack = (STTrack *) fTrackPreArray -> At(genfitID);
    auto pdg = preTrack -> GetPID();

    Int_t trackID = fTrackArray -> GetEntriesFast();
    STTrack *recoTrack = (STTrack *) fTrackArray -> ConstructedAt(trackID);
    recoTrack -> SetTrackID(trackID);
    recoTrack -> SetHelixID(helixTrack -> GetTrackID());

    auto clusterArray = helixTrack -> GetClusterArray();
    for (auto cluster : *clusterArray) {
      if (cluster -> IsStable()) {
        recoTrack -> AddHitID(cluster -> GetClusterID());
      }
    }

    Int_t trackCandID = fTrackCandArray -> GetEntriesFast();
    STTrackCandidate *candTrack = (STTrackCandidate *) fTrackCandArray -> ConstructedAt(trackCandID);
    candTrack -> SetCharge(helixTrack -> Charge());

    genfit::Track *track = fGenfitTest -> FitTrack(helixTrack, pdg);
    if (track == nullptr)
      continue;

    fGenfitTest -> SetTrackParameters(candTrack, track);
    fGenfitTest -> CalculatedEdx2(track, candTrack, helixTrack);

    recoTrack -> SetIsFitted();
    recoTrack -> SetTrackCandidate(candTrack);
    genfitTrackArray.push_back(track);
fGenfitTest -> CalculatedEdx2(track, recoTrack, helixTrack);

    helixTrack -> SetGenfitID(recoTrack -> GetTrackID());
    helixTrack -> SetGenfitMomentum(candTrack -> GetP());
  }

  LOG(INFO) << Space() << "STTrack " << fTrackArray -> GetEntriesFast() << FairLogger::endl;

  if (genfitTrackArray.size() < 2)
    return;

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
