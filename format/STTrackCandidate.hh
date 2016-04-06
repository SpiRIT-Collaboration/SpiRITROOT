#ifndef STTRACKCANDIDATE
#define STTRACKCANDIDATE

#include "Rtypes.h"
#include "TVector3.h"

#include <vector>

class STTrackCandidate {
  public:
    STTrackCandidate();
    ~STTrackCandidate() {};

    void SetPID(Int_t value);
    void SetMass(Double_t value);
    void SetCharge(Int_t value);

    void SetVertex(TVector3 vector);
    void SetVertex(Double_t x, Double_t y, Double_t z);

    void SetBeamVertex(TVector3 vector);
    void SetBeamVertex(Double_t x, Double_t y, Double_t z);

    void SetMomentum(TVector3 vector);
    void SetMomentum(Double_t px, Double_t py, Double_t pz);

    void AdddEdx(Double_t value);

    void SetTrackLength(Double_t value);
    void SetTotaldEdx(Double_t value);

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

    TVector3 GetMomentum();
    Double_t GetP();
    Double_t GetPt();
    Double_t GetPx();
    Double_t GetPy();
    Double_t GetPz();

    Double_t GetTrackLength();
    std::vector<Double_t> *GetdEdxArray();
    Double_t GetTotaldEdx();

    Double_t GetChi2();
    Int_t GetNDF();

  private:
    Int_t fPID;
    Double_t fMass;
    Int_t fCharge;

    Double_t fVx;
    Double_t fVy;
    Double_t fVz;

    Double_t fBeamVx;
    Double_t fBeamVy;
    Double_t fBeamVz;

    Double_t fPx;
    Double_t fPy;
    Double_t fPz;

    Double_t fTrackLength;
    std::vector<Double_t> fdEdxArray;
    Double_t fdEdxTotal;

    Double_t fChi2;
    Int_t fNDF;

  ClassDef(STTrackCandidate, 1)
};

#endif
