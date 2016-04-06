/**
 * @brief STTrack Class
 *
 * @author JungWoo Lee
 */

#include "STTrack.hh"

ClassImp(STTrack)

STTrack::STTrack() 
{
}

void STTrack::SetParentID(Int_t value)                   { fParentID = value; }
void STTrack::SetTrackID(Int_t value)                    { fTrackID = value; }
void STTrack::SetRiemannID(Int_t value)                  { fRiemannID = value; }

void STTrack::SetIsFitted(Bool_t value)                  { fIsFitted = value; }

void STTrack::AddHitID(Int_t value)                      { fHitIDArray.push_back(value); }
void STTrack::AddTrackCandidate(STTrackCandidate *track) { fTrackCandidateArray.push_back(track); }

Int_t STTrack::GetParentID()                    { return fParentID; }
Int_t STTrack::GetTrackID()                     { return fTrackID; }
Int_t STTrack::GetRiemannID()                   { return fRiemannID; }

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
