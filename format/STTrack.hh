/**
 * @brief STTrack Class
 *
 * @author JungWoo Lee
 */

#ifndef STTRACK
#define STTRACK

#include "TVector3.h"

#include "STTrackCandidate.hh"

#include <vector>

class STTrack : public STTrackCandidate {
  public:
    STTrack();
    virtual ~STTrack() {};

    virtual void Clear(Option_t *option = "");
    virtual void Print(Option_t *option="") const;
    void DeleteCandidates();

    void SetParentID(Int_t value);
    void SetTrackID(Int_t value);
    void SetRiemannID(Int_t value);
    void SetHelixID(Int_t value);

    void SetIsFitted(Bool_t value = kTRUE);

    void AddHitID(Int_t value);
    void AddTrackCandidate(STTrackCandidate *track);
    void DetermineBestCandidate();

    Int_t GetParentID();
    Int_t GetTrackID();
    Int_t GetRiemannID();
    Int_t GetHelixID();

    Bool_t IsFitted();

    std::vector<Int_t> *GetHitIDArray();
    std::vector<STTrackCandidate *> *GetTrackCandidateArray();

    Int_t GetNumTrackCandidates();
    STTrackCandidate *GetTrackCandidate(Int_t idx);

    Bool_t SelectTrackCandidate(Int_t idx);
    void SelectTrackCandidate(STTrackCandidate *track);

  private:
    Int_t fParentID;
    Int_t fTrackID;
    Int_t fSubTrackID;

    Bool_t fIsFitted;

    std::vector<Int_t> fHitIDArray;
    std::vector<STTrackCandidate *> fTrackCandidateArray;

  ClassDef(STTrack, 4)
};

#endif
