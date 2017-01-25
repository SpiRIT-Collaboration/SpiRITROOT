#include "STGenfitETask.hh"
#include "STDatabasePDG.hh"
#include "STTrack.hh"
#include "STVertex.hh"

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
  fPDGDB = STDatabasePDG::Instance();
}

STGenfitETask::STGenfitETask(Bool_t persistence)
: STRecoTask("GENFIT Task", 1, persistence)
{
  fPDGDB = STDatabasePDG::Instance();
}

STGenfitETask::~STGenfitETask()
{
}

void STGenfitETask::SetIterationCut(Int_t min, Int_t max)
{
  fMinIterations = min;
  fMaxIterations = max;
}

void STGenfitETask::SetClusteringType(Int_t type) { fClusteringType = type; }

InitStatus
STGenfitETask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fHelixTrackArray = (TClonesArray *) fRootManager -> GetObject("STHelixTrack");
  if (fHelixTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find STHelixTrack array!" << FairLogger::endl;
    return kERROR;
  }

  fTrackArray = new TClonesArray("STTrack");
  fRootManager -> Register("STTrack", "SpiRIT", fTrackArray, fIsPersistence);

  fTrackCandArray = new TClonesArray("STTrackCandidate");

  fGenfitTest = new STGenfitTestE();
  fGenfitTest -> SetMinIterations(fMinIterations);
  fGenfitTest -> SetMaxIterations(fMaxIterations);

  fVertexArray = new TClonesArray("STVertex");
  fRootManager -> Register("STVertexPre", "SpiRIT", fVertexArray, fIsPersistence);

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

void STGenfitETask::Exec(Option_t *opt)
{
  fTrackArray -> Clear("C");
  fTrackCandArray -> Clear("C");
  fVertexArray -> Delete();

  if (fEventHeader -> IsBadEvent())
    return;

  fGenfitTest -> Init();

  Int_t numTrackCand = fHelixTrackArray -> GetEntriesFast();
  for (Int_t iHelixTrack = 0; iHelixTrack < numTrackCand; iHelixTrack++)
  {
    STHelixTrack *helixTrack = (STHelixTrack *) fHelixTrackArray -> At(iHelixTrack);
    if (helixTrack -> IsHelix() == false)
      continue;

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

    auto pdgList = fPDGDB -> GetPDGCandidateArray();
    for (auto pdg : *pdgList)
    {
      Int_t trackCandID = fTrackCandArray -> GetEntriesFast();
      STTrackCandidate *candTrack = (STTrackCandidate *) fTrackCandArray -> ConstructedAt(trackCandID);
      candTrack -> SetCharge(helixTrack -> Charge());

      genfit::Track *track = fGenfitTest -> FitTrack(candTrack, helixTrack, pdg);
      if (track == nullptr)
        continue;

      fGenfitTest -> SetTrackParameters(candTrack, track);
      if (candTrack -> GetPVal() <= 0)
        continue;

      candTrack -> SetGenfitTrack(track);
      recoTrack -> AddTrackCandidate(candTrack);
    }

    Double_t bestPVal = 0;
    STTrackCandidate *bestCandTrack = nullptr;

    auto candArray = recoTrack -> GetTrackCandidateArray();
    for (auto candTrack : *candArray)
    {
      if (candTrack -> GetPVal() > bestPVal) {
        bestPVal = candTrack -> GetPVal();
        bestCandTrack = candTrack;
      }

      if (fClusteringType == 2)
        fGenfitTest -> CalculatedEdx2(candTrack -> GetGenfitTrack(), candTrack, helixTrack);
      else
        fGenfitTest -> CalculatedEdx(candTrack -> GetGenfitTrack(), candTrack, helixTrack);
    }

    if (bestCandTrack == nullptr) {
      fTrackArray -> Remove(recoTrack);
      continue;
    }

    recoTrack -> SetIsFitted();
    recoTrack -> SetTrackCandidate(bestCandTrack);

    if (fClusteringType == 2)
      fGenfitTest -> CalculatedEdx2(recoTrack -> GetGenfitTrack(), recoTrack, helixTrack);
    else
      fGenfitTest -> CalculatedEdx(recoTrack -> GetGenfitTrack(), recoTrack, helixTrack);

    helixTrack -> SetGenfitID(recoTrack -> GetTrackID());
    helixTrack -> SetIsGenfitTrack();
    helixTrack -> SetGenfitMomentum(bestCandTrack -> GetP());
  }
  fTrackArray -> Compress();

  Int_t numRecoTracks = fTrackArray -> GetEntriesFast();

  LOG(INFO) << Space() << "STTrack " << numRecoTracks << FairLogger::endl;

  vector<genfit::Track *> genfitTrackArray;
  for (auto iTrack = 0; iTrack < numRecoTracks; ++iTrack) {
    auto recoTrack = (STTrack *) fTrackArray -> At(iTrack);
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
