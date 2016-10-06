#ifndef STTRACKCANDIDATE
#define STTRACKCANDIDATE

#include "TObject.h"
#include "TVector3.h"
#include "TMatrixDSym.h"

#include <vector>

class STTrackCandidate : public TObject {
  public:
    STTrackCandidate();
    virtual ~STTrackCandidate() {};

    virtual void Clear(Option_t *option = "");

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

    void SetPVal(Double_t value);

    void SetCovSeed(TMatrixDSym covSeed);

    Int_t GetPID();      ///< NOT SET IN v1
    Double_t GetMass();  ///< NOT SET IN v1
    Int_t GetCharge();   ///< NOT SET IN v1

    Double_t DistanceToPrimaryVertex(TVector3 vertex);

    TVector3 GetVertex();  ///< Closest point to the vertex (if found).
    Double_t GetVx();
    Double_t GetVy();
    Double_t GetVz();
    TVector3 GetTargetPlaneVertex();  ///< Vertex on target plane
    TVector3 GetBeamVertex();  ///< Same as GetVertexOnTargetPlane();
    Double_t GetBeamVx();
    Double_t GetBeamVy();
    Double_t GetBeamVz();
    TVector3 GetTargetPlaneMomentum();  ///< Vertex on target plane
    TVector3 GetBeamMomentum();  ///< Same as GetTargetPlaneMomentum()
    Double_t GetBeamMomx();
    Double_t GetBeamMomy();
    Double_t GetBeamMomz();

    TVector3 GetKyotoLHit();   ///< Hit position in left KYOTO plane
    Double_t GetKyotoLHitX();
    Double_t GetKyotoLHitY();
    Double_t GetKyotoLHitZ();
    TVector3 GetKyotoRHit();   ///< Hit position in right KYOTO plane
    Double_t GetKyotoRHitX();
    Double_t GetKyotoRHitY();
    Double_t GetKyotoRHitZ();
    TVector3 GetKatanaHit();   ///< Hit position in KATANA plane
    Double_t GetKatanaHitX();
    Double_t GetKatanaHitY();
    Double_t GetKatanaHitZ();

    TVector3 GetMomentum() const; ///< Momentum vector
    Double_t GetP() const;        ///< Magnitude of momentum
    Double_t GetPt();             ///< Transversal momentum
    Double_t GetPx();
    Double_t GetPy();
    Double_t GetPz();

    Double_t GetTrackLength();
    std::vector<Double_t> *GetdEdxArray();  ///< Array of dE/dx of cluster
    Double_t GetTotaldEdx();                ///< [total charge] / [total length]

    Double_t GetRiemanndEdx();  ///< Do not use in v1.
    Double_t GetChi2();
    Int_t GetNDF();

    Double_t GetPVal();

    /**
     * Calculate dEdx with applied cuts from fdEdxArray.
     * Even if the condition is not satisfied with the cut,
     * dEdx calculation is done and applied condition satus
     * is noticed by the return value.
     * @param return  2: All cuts satistified and applied.
     *                1: lowCut not satistified and removed.
     *                0: lowCut, highCut not satistified and removed.
     *               -1: No cut is applied.
     * Input parameters:
     * @param dEdx           Returns calculated dEdx.
     * @param numUsedPoints  Returns number of used points to calculate dEdx.
     * @param lowCut         Lower cut of dEdx in ratio(0~1).
     * @param highCut        High  cut of dEdx in ratio(0~1).
     * @param numCut         Cut for number of used points to calculate dEdx.
     */
    Int_t GetdEdxWithCut(Double_t &dEdx, Int_t &numUsedPoints, Double_t lowCut, Double_t highCut, Int_t numCut);

    /**
     * dE/dx with all cut, return -1 if number of (dE/dx point) is not enough.
     */
    Double_t GetdEdxWithCut(Double_t lowCut, Double_t highCut);

    const TMatrixDSym &GetCovSeed() const;

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

    Double_t fPVal;

    TMatrixDSym fCovSeed;

  ClassDef(STTrackCandidate, 4)
};

#endif
