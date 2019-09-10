#include "STMCTruthTask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

#include "STMCTrack.h"
#include "STRecoTrack.hh"
#include "STMCRecoMatching.hh"

#include <iostream>
using namespace std;

ClassImp(STMCTruthTask)

STMCTruthTask::STMCTruthTask()
: STRecoTask("MC Truth Task", 1, false)
{
}

STMCTruthTask::STMCTruthTask(Bool_t persistence)
: STRecoTask("MC Truth Task", 1, persistence)
{
}

STMCTruthTask::~STMCTruthTask()
{
}

InitStatus STMCTruthTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fMCTrackArray = (TClonesArray*) fRootManager -> GetObject("PrimaryTrack");
  if (fMCTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find MCPointArray array!" << FairLogger::endl;
    return kERROR;
  }

  fRecoTrackArray = (TClonesArray*) fRootManager -> GetObject("STRecoTrack");
  if (fRecoTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find RecoTrack array!" << FairLogger::endl;
    return kERROR;
  }

  fMCArray = new TClonesArray("STMCRecoMatching", 100);
  fRecoArray = new TClonesArray("STMCRecoMatching", 100);

  fMCTruthArray = new TClonesArray("STMCRecoMatching", 100);
  fRootManager -> Register("STMCRecoMatching", "SpiRIT", fMCTruthArray, fIsPersistence);

  fMCVertex = new TClonesArray("STVertex");
  fRootManager -> Register("STVertex", "SpiRIT", fMCVertex, fIsPersistence);
  fRootManager -> Register("STMCTrack", "SpiRIT", fMCTrackArray, fIsPersistence);
  
  return kSUCCESS;
}

void STMCTruthTask::Exec(Option_t *opt)
{
  if (fEventHeader -> IsBadEvent())
    return;

  fMCArray -> Clear();
  fRecoArray -> Clear();
  fMCTruthArray -> Clear();

  auto numMCTracks = fMCTrackArray -> GetEntries();
  for (auto iTrack = 0; iTrack < numMCTracks; iTrack++)
  {
    auto mcTrack = (STMCTrack *) fMCTrackArray -> At(iTrack);
    if (mcTrack -> GetPdgCode() == 2112)
      continue;

    TVector3 momMC;
    mcTrack -> GetMomentum(momMC);
    momMC = 1000*momMC;
    
    TVector3 vertex;
    mcTrack -> GetStartVertex(vertex);
    auto vert = (STVertex *) fMCVertex -> ConstructedAt(fMCVertex -> GetEntries());
    vert -> SetPos(vertex);

    auto mc = (STMCRecoMatching *) fMCArray -> ConstructedAt(fMCArray -> GetEntries());
    mc -> SetMCID(iTrack);
    mc -> SetMCMomentum(momMC);
    mc -> SetIsNotFound();
    mc -> SetMCPDGCode(mcTrack -> GetPdgCode());
  }

  auto numRecoTracks = fRecoTrackArray -> GetEntries();
  for (auto iTrack = 0; iTrack < numRecoTracks; iTrack++)
  {
    auto recoTrack = (STRecoTrack *) fRecoTrackArray -> At(iTrack);
    if (recoTrack -> GetParentID() != 0)
      continue;

    //auto vtrack = recoTrack -> GetVertex();
    auto momReco = recoTrack -> GetMomentum();

    auto reco = (STMCRecoMatching *) fRecoArray -> ConstructedAt(fRecoArray -> GetEntries());
    reco -> SetRecoID(iTrack);
    reco -> SetRecoMomentum(momReco);
    reco -> SetIsFake();
  }

  auto numMC = fMCArray -> GetEntries();

  Int_t countMatch = 0;
  Double_t cutArray[] = {.001, .002, .003, .004, .006, .008, .01, .015, .02, .03, .046, .08, .1};
  for (auto iCut : cutArray)
  {
    for (auto iMC = 0; iMC < numMC; iMC++)
    {
      auto mc = (STMCRecoMatching *) fMCArray -> At(iMC);
      if (mc -> IsMatched())
        continue;

      auto momMC = mc -> GetMCMomentum();

      STMCRecoMatching *bestMatching = nullptr;
      Int_t bestMatchingIdx = -1;
      Double_t bestMatchingV = 0;
      TVector3 bestMatchingMom;
      auto numReco = fRecoArray -> GetEntries();
      for (auto iReco = 0; iReco < numReco; iReco++)
      {
        auto reco = (STMCRecoMatching *) fRecoArray -> At(iReco);

        if (reco -> IsMatched())
          continue;

        auto momReco = reco -> GetRecoMomentum();

        auto momDiff = momMC.Mag() - momReco.Mag() / momMC.Mag();
        auto v = (momMC.Dot(momReco))/momMC.Mag()/momReco.Mag();

        if (v > 1 - iCut && bestMatchingV < v) {
          bestMatchingV = v;
          bestMatching = reco;
          bestMatchingIdx = iReco;
          bestMatchingMom = momReco;
        }
      }


      if (bestMatching != nullptr) {
        bestMatching -> SetIsMatched();
        mc -> SetIsMatched();

        auto truth = (STMCRecoMatching *) fMCTruthArray -> ConstructedAt(fMCTruthArray -> GetEntries());
        truth -> SetIsMatched();
        truth -> Init(iMC, bestMatchingIdx, mc -> GetMCPDGCode(), momMC, bestMatchingMom);
        countMatch++;
      }
    }
  }

  for (auto iMC = 0; iMC < numMC; iMC++)
  {
    auto mc = (STMCRecoMatching *) fMCArray -> At(iMC);
    if (mc -> IsMatched())
      continue;

    auto truth = (STMCRecoMatching *) fMCTruthArray -> ConstructedAt(fMCTruthArray -> GetEntries());
    truth -> SetIsNotFound();
    truth -> Init(iMC, -1, mc -> GetMCPDGCode(), mc -> GetMCMomentum(), TVector3(0,0,0));
  }

  auto numReco = fRecoArray -> GetEntries();
  for (auto iReco = 0; iReco < numReco; iReco++)
  {
    auto reco = (STMCRecoMatching *) fRecoArray -> At(iReco);
    if (reco -> IsMatched())
      continue;

    auto truth = (STMCRecoMatching *) fMCTruthArray -> ConstructedAt(fMCTruthArray -> GetEntries());
    truth -> SetIsFake();
    truth -> Init(-1, iReco, 0, TVector3(0,0,0), reco -> GetRecoMomentum());
  }

  LOG(INFO) << Space() << "MC Matching: " << countMatch << " / " << fMCArray -> GetEntries() << FairLogger::endl;
}
