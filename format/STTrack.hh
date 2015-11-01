/**
 * @brief STTrack Class
 *
 * @author JungWoo Lee
 */

#ifndef STTRACK
#define STTRACK

#include "TObject.h"
#include "TVector3.h"

#include <vector>

class STTrack : public TObject {
  public:
    STTrack();
    virtual ~STTrack() {};

    void SetParentID(Int_t value);
    void SetTrackID(Int_t value);
    void SetRiemannID(Int_t value);

    void SetPID(Int_t value);
    void SetMass(Double_t value);

    void SetVertex(TVector3 vector);
    void SetVertex(Double_t x, Double_t y, Double_t z);

    void SetMomentum(TVector3 vector);
    void SetMomentum(Double_t px, Double_t py, Double_t pz);

    void SetCharge(Int_t value);
    void AdddEdx(Double_t value);

    void SetChi2(Double_t value);
    void SetTrackLength(Double_t value);
    void SetNDF(Int_t value);

    void AddHitID(Double_t value);

    Int_t GetParentID();
    Int_t GetTrackID();
    Int_t GetRiemannID();

    Int_t GetPID();
    Double_t GetMass();

    TVector3 GetVertex();
    Double_t GetVx();
    Double_t GetVy();
    Double_t GetVz();

    TVector3 GetMomentum();
    Double_t GetP();
    Double_t GetPt();
    Double_t GetPx();
    Double_t GetPy();
    Double_t GetPz();

    Int_t GetCharge();
    Double_t GetTrackLength();
    std::vector<Double_t> *GetdEdxArray();

    Double_t GetChi2();
    Int_t GetNDF();

    std::vector<Double_t> *GetHitIDArray();

  private:
    Int_t fParentID;
    Int_t fTrackID;
    Int_t fRiemannID;

    Int_t fPID;
    Double_t fMass;

    Double_t fVx;
    Double_t fVy;
    Double_t fVz;

    Double_t fPx;
    Double_t fPy;
    Double_t fPz;

    Int_t fCharge;
    Double_t fTrackLength;
    std::vector<Double_t> fdEdx;

    Double_t fChi2;
    Int_t fNDF;

    std::vector<Int_t> fHitIDArray;

  ClassDef(STTrack, 1)
};

#endif
