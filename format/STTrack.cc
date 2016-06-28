/**
 * @brief STTrack Class
 *
 * @author JungWoo Lee
 */

#include "STTrack.hh"

ClassImp(STTrack)

STTrack::STTrack() 
{
  Clear();
}

void STTrack::SetParentID(Int_t value)                   { fParentID = value; }
void STTrack::SetTrackID(Int_t value)                    { fTrackID = value; }
void STTrack::SetRiemannID(Int_t value)                  { fSubTrackID = value; }
void STTrack::SetHelixID(Int_t value)                    { fSubTrackID = value; }

void STTrack::SetIsFitted(Bool_t value)                  { fIsFitted = value; }

void STTrack::AddHitID(Int_t value)                      { fHitIDArray.push_back(value); }
void STTrack::AddTrackCandidate(STTrackCandidate *track) { fTrackCandidateArray.push_back(track); }

void STTrack::DetermineBestCandidate()
{
  Double_t bestChi2 = 1.e10;
  STTrackCandidate *bestCandidate = nullptr;
  for (auto cand : fTrackCandidateArray) {
    if (cand -> GetChi2() < bestChi2) {
      bestChi2 = cand -> GetChi2();
      bestCandidate = cand;
    }
  }
  if (bestCandidate != nullptr)
    SetTrackCandidate(bestCandidate);
}

void STTrack::Clear(Option_t *option)
{
  STTrackCandidate::Clear();

  fParentID = -99999;
  fTrackID = -99999;
  fSubTrackID = -99999;

  fIsFitted = kFALSE;

  fHitIDArray.clear();

  if (TString(option) == "C")
    DeleteCandidates();
  else
    fTrackCandidateArray.clear();
}

void STTrack::DeleteCandidates()
{
  for (auto cand : fTrackCandidateArray)
    delete cand;

  fTrackCandidateArray.clear();
}

Int_t STTrack::GetParentID()                    { return fParentID; }
Int_t STTrack::GetTrackID()                     { return fTrackID; }
Int_t STTrack::GetRiemannID()                   { return fSubTrackID; }
Int_t STTrack::GetHelixID()                     { return fSubTrackID; }

Bool_t STTrack::IsFitted()                      { return fIsFitted; }

std::vector<Int_t> *STTrack::GetHitIDArray()    { return &fHitIDArray; }
std::vector<STTrackCandidate *> *STTrack::GetTrackCandidateArray() { return &fTrackCandidateArray; }

Int_t STTrack::GetNumTrackCandidates() { return fTrackCandidateArray.size(); }

STTrackCandidate *STTrack::GetTrackCandidate(Int_t idx)
{
  return fTrackCandidateArray.size() > idx ? fTrackCandidateArray[idx] : nullptr;
}

Bool_t STTrack::SelectTrackCandidate(Int_t idx)
{
  if (fTrackCandidateArray.size() < idx) 
    return kFALSE;

  SelectTrackCandidate(fTrackCandidateArray[idx]);
  return kTRUE;
}

void STTrack::SelectTrackCandidate(STTrackCandidate *track) { SetTrackCandidate(track); }
