#include "STRecoTrackCandList.hh"
#include <iostream>

ClassImp(STRecoTrackCandList)

STRecoTrackCandList::STRecoTrackCandList() 
{
  for (auto iPID = 0; iPID < NUMSTPID; iPID++) {
    auto cand = new STRecoTrackCand();
    STPID::PID pid = static_cast<STPID::PID>(iPID);
    cand -> SetPID(pid);
    fCandArray.push_back(*cand);
  }

  Clear();
}

void STRecoTrackCandList::Clear(Option_t *option)
{
  fHitIDArray.clear();
  for (auto iPID = 0; iPID < NUMSTPID; iPID++)
    fCandArray.at(iPID).Clear();

  fBestPID = STPID::kNon;
}

void STRecoTrackCandList::Print(Option_t *option) const
{
  for (auto iPID = 0; iPID < NUMSTPID; iPID++)
    fCandArray.at(iPID).Print();
}

STRecoTrackCand *STRecoTrackCandList::GetRecoTrackCand(Int_t pid)
{
  if (pid == -1)
    return nullptr;
  return &fCandArray.at(pid);
}
