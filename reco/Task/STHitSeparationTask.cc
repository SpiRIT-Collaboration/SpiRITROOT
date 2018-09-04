#include "STHitSeparationTask.hh"

#include "FairRun.h"
#include "FairRootManager.h"
#include "FairRuntimeDb.h"

#include "STHit.hh"
#include "STHelixTrack.hh"
#include "STVertex.hh"
#include "STRecoTrack.hh"

ClassImp(STHitSeparationTask)

STHitSeparationTask::STHitSeparationTask(Bool_t two)
: STRecoTask("Hit Separation Task", 1, false)
{
  fTwoHitEfficiency = two;
}

STHitSeparationTask::~STHitSeparationTask()
{
}

InitStatus STHitSeparationTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  if (fRecoHeader != nullptr)
    fSplit = fRecoHeader -> GetParInt("split");

  fHitArray = (TClonesArray *) fRootManager -> GetObject("STHit");
  if (fHitArray == nullptr) {
    LOG(ERROR) << "Cannot find STHit array!" << FairLogger::endl;
    return kERROR;
  }

  fHelixTrackArray = (TClonesArray *) fRootManager -> GetObject("STHelixTrack");
  if (fHelixTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find STHelixTrack array!" << FairLogger::endl;
    return kERROR;
  }

  fRecoTrackArray = (TClonesArray *) fRootManager -> GetObject("STRecoTrack");
  if (fRecoTrackArray == nullptr) {
    LOG(ERROR) << "Cannot find STRecoTrack array!" << FairLogger::endl;
    return kERROR;
  }

  fVertexArray = (TClonesArray *) fRootManager -> GetObject("STVertex");
  if (fVertexArray == nullptr) {
    LOG(ERROR) << "Cannot find STVertex array!" << FairLogger::endl;
    return kERROR;
  }

  fPPMapTrackHit = new STPadPlaneMap();
  fTrackHits = new vector<STHit *>;

  return kSUCCESS;
}

void STHitSeparationTask::Exec(Option_t *opt)
{
  ++fEventID;

  fPPMapTrackHit -> Clear();

  if (fVertexArray -> GetEntries() != 1)
    return;

  auto vz = ((STVertex*)fVertexArray->At(0))->GetPos().Z();
  if (vz<-20||vz>0)
    return;

  //////////////////////////////////////////////////////////////////////////////
  //                      True hit separation from track                      //
  //////////////////////////////////////////////////////////////////////////////

  auto numTracks = fHelixTrackArray -> GetEntries();
  for (auto iTrack = 0; iTrack < numTracks; ++iTrack)
  {
    auto helix = (STHelixTrack *) fHelixTrackArray -> At(iTrack);
    auto genfitID = helix -> GetGenfitID();
    if (genfitID < 0)
      continue;

    auto recoTrack = (STRecoTrack *) fRecoTrackArray -> At(genfitID);
    if (recoTrack -> GetCharge() == 0)
      continue;

    auto dedx = recoTrack -> GetdEdxWithCut(0,0.7);
    if (dedx <= 0)
      continue;

    auto nn = recoTrack -> GetdEdxPointArray() -> size();
    if (nn < 30)
      continue;

    auto trackHitArray = helix -> GetPadHitArray();
    for (auto hit : *trackHitArray) {
      fPPMapTrackHit -> AddHit(hit);
    }
  }

  //////////////////////////////////////////////////////////////////////////////
  //                       Reconstructed Hit Separation                       //
  //////////////////////////////////////////////////////////////////////////////

  if (fTwoHitEfficiency) {
    for (fRow = 0; fRow < 108; ++fRow) {
      for (fLayer = 0; fLayer < 112; ++fLayer)
      {
        auto padTH = fPPMapTrackHit -> GetPad(fRow, fLayer);

        fTrackHits -> clear();
        padTH -> GetHits(fTrackHits);

        auto numTrackHits = fTrackHits->size();
        if (numTrackHits != 2)
          continue;

        sort(fTrackHits->begin(), fTrackHits->end(), STHitSortY());
        auto trackHit1 = fTrackHits -> at(0);
        auto trackHit2 = fTrackHits -> at(1);
        fDTrue = trackHit1 -> GetY() - trackHit2 -> GetY();

        fTrueID1 = trackHit1 -> GetHitID();
        fTrueID2 = trackHit2 -> GetHitID();
        fRecoID1 = trackHit1 -> GetTrackID();
        fRecoID2 = trackHit2 -> GetTrackID();

        fY = trackHit1 -> GetY();
        fNumPad1 = trackHit1 -> GetNDF();
        fNumPad2 = trackHit2 -> GetNDF();

        if (fRecoID1==-1)
          continue;

        auto hit1 = (STHit *) fHitArray -> At(fRecoID1);

        if (fRecoID2==-1) { // first hit exist but not the second one
          fFound = false;
          fQ1 = hit1 -> GetCharge();
          fQ2 = trackHit2 -> GetCharge(); 
          fDReco = -1;
        }
        else { // both the frist and second hit exist
          auto hit2 = (STHit *) fHitArray -> At(fRecoID2);

          fFound = true;
          fQ1 = hit1 -> GetCharge();
          fQ2 = hit2 -> GetCharge();
          fDReco = hit1 -> GetY() - hit2 -> GetY();
        }
        fFoundTree -> Fill();
      }
    }
  }
  else {
    for (fRow = 0; fRow < 108; ++fRow) {
      for (fLayer = 0; fLayer < 112; ++fLayer)
      {
        auto padTH = fPPMapTrackHit -> GetPad(fRow, fLayer);

        fTrackHits -> clear();
        padTH -> GetHits(fTrackHits);

        auto numTrackHits = fTrackHits->size();
        if (numTrackHits != 1)
          continue;

        sort(fTrackHits->begin(), fTrackHits->end(), STHitSortY());
        auto trackHit1 = fTrackHits -> at(0);
        fY = trackHit1 -> GetY();
        fNumPad1 = trackHit1 -> GetNDF();

        fRecoID1 = trackHit1 -> GetTrackID();
        if (fRecoID1==-1) {
          fFound = false;
          fQ1 = trackHit1 -> GetCharge();
        }
        else {
          fFound = true;
          auto hit1 = (STHit *) fHitArray -> At(fRecoID1);
          fQ1 = hit1 -> GetCharge();
        }

        fFoundTree -> Fill();
      }
    }
  }

  return;
}

void STHitSeparationTask::SetHitSeparationFile(TString fileName)
{
  fFile = new TFile(fileName,"recreate");

  fFoundTree = new TTree("found","");
  fFoundTree -> Branch("split", &fSplit);
  fFoundTree -> Branch("event", &fEventID);
  fFoundTree -> Branch("found", &fFound);
  fFoundTree -> Branch("npad1", &fNumPad1);
  fFoundTree -> Branch("row",   &fRow);
  fFoundTree -> Branch("layer", &fLayer);
  fFoundTree -> Branch("y",     &fY);
  fFoundTree -> Branch("q1",    &fQ1);
  fFoundTree -> Branch("trueid1", &fTrueID1);
  fFoundTree -> Branch("recoid1", &fRecoID1);
  if (fTwoHitEfficiency) {
    fFoundTree -> Branch("npad2", &fNumPad2);
    fFoundTree -> Branch("dtrue", &fDTrue);
    fFoundTree -> Branch("dreco", &fDReco);
    fFoundTree -> Branch("q2",    &fQ2);
    fFoundTree -> Branch("trueid2", &fTrueID2);
    fFoundTree -> Branch("recoid2", &fRecoID2);
  }
}

void STHitSeparationTask::WriteHitSeparationFile()
{
  fFile -> cd();
  fFoundTree -> Write();
}
