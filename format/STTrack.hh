/**
 * @brief STTrack Class
 *
 * @author JungWoo Lee
 */

#ifndef STTRACK
#define STTRACK

#include "TObject.h"
#include "TVector3.h"

#include "STTrackCandidate.hh"

#include <vector>

class STTrack : public TObject {
  public:
    STTrack();
    virtual ~STTrack() {};

    void SetParentID(Int_t value);
    void SetTrackID(Int_t value);
    void SetRiemannID(Int_t value);

    void SetIsFitted(Bool_t value = kTRUE);

    void SetPID(Int_t value);
    void SetMass(Double_t value);
    void SetCharge(Int_t value);

    void SetVertex(TVector3 vector);
    void SetVertex(Double_t x, Double_t y, Double_t z);

    void SetBeamVertex(TVector3 vector);
    void SetBeamVertex(Double_t x, Double_t y, Double_t z);

    void SetKyotoLHit(TVector3 vector);
    void SetKyotoLHit(Double_t x, Double_t y, Double_t z);

    void SetMomentum(TVector3 vector);
    void SetMomentum(Double_t px, Double_t py, Double_t pz);

    void AdddEdx(Double_t value);

    void SetTrackLength(Double_t value);
    void SetTotaldEdx(Double_t value);

    void SetChi2(Double_t value);
    void SetNDF(Int_t value);

    void AddHitID(Int_t value);

    void AddTrackCandidate(STTrackCandidate *track);

    Int_t GetParentID();
    Int_t GetTrackID();
    Int_t GetRiemannID();

    Bool_t IsFitted();

    Int_t GetPID();
    Double_t GetMass();
    Int_t GetCharge();

    TVector3 GetVertex();
    Double_t GetVx();
    Double_t GetVy();
    Double_t GetVz();
    TVector3 GetBeamVertex();
    Double_t GetBeamVx();
    Double_t GetBeamVy();
    Double_t GetBeamVz();

    TVector3 GetKyotoLHit();
    Double_t GetKyotoLHitX();
    Double_t GetKyotoLHitY();
    Double_t GetKyotoLHitZ();

    TVector3 GetMomentum();
    Double_t GetP();
    Double_t GetPt();
    Double_t GetPx();
    Double_t GetPy();
    Double_t GetPz();

    std::vector<Double_t> *GetdEdxArray();

    Double_t GetTrackLength();
    Double_t GetTotaldEdx();

    Double_t GetChi2();
    Int_t GetNDF();

    std::vector<Int_t> *GetHitIDArray();

    std::vector<STTrackCandidate *> *GetTrackCandidateArray();

    Int_t GetNumTrackCandidates();
    STTrackCandidate *GetTrackCandidate(Int_t idx);

    Bool_t SelectTrackCandidate(Int_t idx);
    void SelectTrackCandidate(STTrackCandidate *track);

  private:
    Int_t fParentID;
    Int_t fTrackID;
    Int_t fRiemannID;

    Bool_t fIsFitted;

    Int_t fPID;
    Double_t fMass;
    Int_t fCharge;

    Double_t fVx;
    Double_t fVy;
    Double_t fVz;

    Double_t fBeamVx;
    Double_t fBeamVy;
    Double_t fBeamVz;

    Double_t fKyotoLx;
    Double_t fKyotoLy;
    Double_t fKyotoLz;

    Double_t fPx;
    Double_t fPy;
    Double_t fPz;

    Double_t fTrackLength;
    std::vector<Double_t> fdEdxArray;
    Double_t fdEdxTotal;

    Double_t fChi2;
    Int_t fNDF;

    std::vector<Int_t> fHitIDArray;

    std::vector<STTrackCandidate *> fTrackCandidateArray;

  ClassDef(STTrack, 2)
};

#endif
