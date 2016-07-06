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
  fGenfitTest -> SetMinIterations(min);
  fGenfitTest -> SetMaxIterations(max);
  fMinIterations = min;
  fMaxIterations = max;
}

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

  fHitClusterArray = (TClonesArray *) fRootManager -> GetObject("STHitCluster");
  if (fHitClusterArray == nullptr) {
    LOG(ERROR) << "Cannot find STHitCluster array!" << FairLogger::endl;
    return kERROR;
  }

  fTrackArray = new TClonesArray("STTrack");
  fRootManager -> Register("STTrack", "SpiRIT", fTrackArray, fIsPersistence);

  fTrackCandArray = new TClonesArray("STTrackCandidate");

  fGenfitTest = new STGenfitTestE();

//  fVertexArray = new TClonesArray("genfit::GFRaveVertex");
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

void STGenfitETask::Exec(Option_t *opt)
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
    if (helixTrack -> IsHelix() == false)
      continue;

    Int_t trackID = fTrackArray -> GetEntriesFast();
    STTrack *recoTrack = (STTrack *) fTrackArray -> ConstructedAt(trackID);
    recoTrack -> SetTrackID(trackID);
    recoTrack -> SetHelixID(helixTrack -> GetTrackID());

    Int_t numHits = helixTrack -> GetNumClusters();
    for (Int_t iHit = 0; iHit < numHits; iHit++)
      recoTrack -> AddHitID(helixTrack -> GetClusterID(iHit));

    vector<genfit::Track *> gfCandTrackArray;
    vector<STTrackCandidate*> recoCandTrackArray;

    auto pdgList = fPDGDB -> GetPDGCandidateArray();
    for (auto pdg : *pdgList)
    {
      Int_t trackCandID = fTrackCandArray -> GetEntriesFast();
      STTrackCandidate *candTrack = (STTrackCandidate *) fTrackCandArray -> ConstructedAt(trackCandID);

      genfit::Track *track = fGenfitTest -> FitTrack(candTrack, fHitClusterArray, helixTrack, pdg);
      if (track != nullptr)
      {
        fGenfitTest -> SetTrack(candTrack, track);
        if (candTrack -> GetPVal() <= 0) {
          continue;
        }
        gfCandTrackArray.push_back(track);
        recoCandTrackArray.push_back(candTrack);
      }
    }

    Double_t bestPVal = 0;
    STTrackCandidate *bestCandTrack = nullptr;
    genfit::Track *bestGFTrack = nullptr;

    for (auto i=0; i<recoCandTrackArray.size(); i++)
    {
      auto candTrack = recoCandTrackArray[i];
      auto gfTrack = gfCandTrackArray[i];

      if (candTrack -> GetPVal() > bestPVal) {
        bestPVal = candTrack -> GetPVal();
        bestCandTrack = candTrack;
        bestGFTrack = gfTrack;
      }
      recoTrack -> AddTrackCandidate(candTrack);
    }

    if (bestCandTrack == nullptr)
      continue;

    recoTrack -> SetIsFitted();
    recoTrack -> SetTrackCandidate(bestCandTrack);
    genfitTrackArray.push_back(bestGFTrack);

    helixTrack -> SetGenfitID(recoTrack -> GetTrackID());
    helixTrack -> SetIsGenfitTrack();
    helixTrack -> SetGenfitMomentum(bestCandTrack -> GetP());
  }
  fTrackArray -> Compress();

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
//    new ((*fVertexArray)[iVert]) genfit::GFRaveVertex(*vertex);
    new ((*fVertexArray)[iVert]) STVertex(*vertex);
    delete vertex;
  }
}
