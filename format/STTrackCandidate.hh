#ifndef STTRACKCANDIDATE
#define STTRACKCANDIDATE

#include "TObject.h"
#include "TVector3.h"

#include <vector>

class STTrackCandidate : public TObject {
  public:
    STTrackCandidate();
    virtual ~STTrackCandidate() {};

    void SetTrackCandidate(STTrackCandidate *track);

    void SetPID(Int_t value);
    void SetMass(Double_t value);
    void SetCharge(Int_t value);

    void SetVertex(TVector3 vector);
    void SetVertex(Double_t x, Double_t y, Double_t z);

    void SetBeamVertex(TVector3 vector);
    void SetBeamVertex(Double_t x, Double_t y, Double_t z);
    void SetBeamMomentum(TVector3 vector);
    void SetBeamMomentum(Double_t x, Double_t y, Double_t z);

    void SetKyotoLHit(TVector3 vector);
    void SetKyotoLHit(Double_t x, Double_t y, Double_t z);
    void SetKyotoRHit(TVector3 vector);
    void SetKyotoRHit(Double_t x, Double_t y, Double_t z);
    void SetKatanaHit(TVector3 vector);
    void SetKatanaHit(Double_t x, Double_t y, Double_t z);

    void SetMomentum(TVector3 vector);
    void SetMomentum(Double_t px, Double_t py, Double_t pz);

    void AdddEdx(Double_t value);

    void SetTrackLength(Double_t value);
    void SetTotaldEdx(Double_t value);

    void SetRiemanndEdx(Double_t value);
    void SetChi2(Double_t value);
    void SetNDF(Int_t value);

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
    TVector3 GetBeamMomentum();
    Double_t GetBeamMomx();
    Double_t GetBeamMomy();
    Double_t GetBeamMomz();

    TVector3 GetKyotoLHit();
    Double_t GetKyotoLHitX();
    Double_t GetKyotoLHitY();
    Double_t GetKyotoLHitZ();
    TVector3 GetKyotoRHit();
    Double_t GetKyotoRHitX();
    Double_t GetKyotoRHitY();
    Double_t GetKyotoRHitZ();
    TVector3 GetKatanaHit();
    Double_t GetKatanaHitX();
    Double_t GetKatanaHitY();
    Double_t GetKatanaHitZ();

    TVector3 GetMomentum() const;
    Double_t GetP() const;
    Double_t GetPt();
    Double_t GetPx();
    Double_t GetPy();
    Double_t GetPz();

    Double_t GetTrackLength();
    std::vector<Double_t> *GetdEdxArray();
    Double_t GetTotaldEdx();

    Double_t GetRiemanndEdx();
    Double_t GetChi2();
    Int_t GetNDF();

  protected:
    Int_t fPID;
    Double_t fMass;
    Int_t fCharge;

    Double_t fVx;
    Double_t fVy;
    Double_t fVz;

    Double_t fBeamVx;
    Double_t fBeamVy;
    Double_t fBeamVz;
    Double_t fBeamMomx;
    Double_t fBeamMomy;
    Double_t fBeamMomz;

    Double_t fKyotoLx;
    Double_t fKyotoLy;
    Double_t fKyotoLz;
    Double_t fKyotoRx;
    Double_t fKyotoRy;
    Double_t fKyotoRz;
    Double_t fKatanax;
    Double_t fKatanay;
    Double_t fKatanaz;

    Double_t fPx;
    Double_t fPy;
    Double_t fPz;

    Double_t fTrackLength;
    std::vector<Double_t> fdEdxArray;
    Double_t fdEdxTotal;

    Double_t fRiemanndEdx;
    Double_t fChi2;
    Int_t fNDF;

  ClassDef(STTrackCandidate, 2)
};

#endif
