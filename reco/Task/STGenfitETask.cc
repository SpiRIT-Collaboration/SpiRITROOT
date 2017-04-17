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
void STGenfitETask::SetConstantField()            { fIsSamurai = kFALSE; }

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
  fRootManager -> Register("STTrackPre", "SpiRIT", fTrackArray, fIsPersistence);

  fTrackCandArray = new TClonesArray("STTrackCandidate");

  fGenfitTest = new STGenfitTestE(fIsSamurai);
  fGenfitTest -> SetMinIterations(fMinIterations);
  fGenfitTest -> SetMaxIterations(fMaxIterations);

  if (fRecoHeader != nullptr) {
    fRecoHeader -> SetPar("genfit_pre_minimumIteration", fMinIterations);
    fRecoHeader -> SetPar("genfit_pre_maximumIteration", fMaxIterations);
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }

  return kSUCCESS;
}

void STGenfitETask::Exec(Option_t *opt)
{
  fTrackArray -> Clear("C");
  fTrackCandArray -> Clear("C");

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

    auto clusterArray = helixTrack -> GetClusterArray();
    for (auto cluster : *clusterArray) {
      if (cluster -> IsStable()) {
        recoTrack -> AddHitID(cluster -> GetClusterID());
      }
    }

    vector<genfit::Track *> gfCandTrackArray;
    vector<STTrackCandidate*> recoCandTrackArray;

    auto pdgList = fPDGDB -> GetPDGCandidateArray();
    for (auto pdg : *pdgList)
    {
      Int_t trackCandID = fTrackCandArray -> GetEntriesFast();
      STTrackCandidate *candTrack = (STTrackCandidate *) fTrackCandArray -> ConstructedAt(trackCandID);
      candTrack -> SetCharge(helixTrack -> Charge());

      /*
      genfit::Track *track0 = fGenfitTest -> FitTrack(candTrack, helixTrack, pdg);
      if (track0 == nullptr)
        continue;

      fGenfitTest -> VarifyClusters(track0, helixTrack);
      */
      genfit::Track *track = fGenfitTest -> FitTrack(helixTrack, pdg);
      if (track == nullptr)
        continue;

      fGenfitTest -> SetTrackParameters(candTrack, track);
      if (candTrack -> GetPVal() <= 0)
        continue;

      gfCandTrackArray.push_back(track);
      recoCandTrackArray.push_back(candTrack);
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

      if (fClusteringType == 2)
        fGenfitTest -> CalculatedEdx2(gfTrack, candTrack, helixTrack);
      else
        fGenfitTest -> CalculatedEdx(gfTrack, candTrack, helixTrack);
    }

    if (bestCandTrack == nullptr) {
      fTrackArray -> Remove(recoTrack);
      continue;
    }

    recoTrack -> SetIsFitted();
    recoTrack -> SetTrackCandidate(bestCandTrack);
    genfitTrackArray.push_back(bestGFTrack);

    if (fClusteringType == 2)
      fGenfitTest -> CalculatedEdx2(bestGFTrack, recoTrack, helixTrack);
    else
      fGenfitTest -> CalculatedEdx(bestGFTrack, recoTrack, helixTrack);

    helixTrack -> SetGenfitID(recoTrack -> GetTrackID());
    helixTrack -> SetIsGenfitTrack();
    helixTrack -> SetGenfitMomentum(bestCandTrack -> GetP());
  }
  fTrackArray -> Compress();

  LOG(INFO) << Space() << "STTrack " << fTrackArray -> GetEntriesFast() << FairLogger::endl;
}
