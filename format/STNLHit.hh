#ifndef STNLHIT_HH
#define STNLHIT_HH

#include <vector>
using std::vector;

#include "TObject.h"
#include "TArrow.h"
#include "ODRFitter.hh"

class STNLHit : public TObject
{
  private:

    // ===========================================================
    // Hit
    // ===========================================================

      Bool_t fIsCluster = false;
       Int_t fID = -1;
       Int_t fBarID = -1;
    Double_t fTOF = -1.;
    Double_t fBeta = -1.;
    TVector3 fPosition = TVector3(-1,-1,-1); ///< local position

    Double_t fQ = 0.;  ///< (Sum of) charge


    // ===========================================================
    // Cluster
    // ===========================================================

    vector<STNLHit *> fHitPtrs; //!
    vector<Int_t> fHitIDs; ///<

    Int_t fN = 0;

    Double_t fEX  = 0;  //! < <x>   Mean value of x
    Double_t fEY  = 0;  //! < <y>   Mean value of y
    Double_t fEZ  = 0;  //! < <z>   Mean value of z
    Double_t fEXX = 0;  //! < <x*x> Mean value of x*x
    Double_t fEYY = 0;  //! < <y*y> Mean value of y*y
    Double_t fEZZ = 0;  //! < <z*z> Mean value of z*z
    Double_t fEXY = 0;  //! < <x*y> Mean value of x*y
    Double_t fEYZ = 0;  //! < <y*z> Mean value of y*z
    Double_t fEZX = 0;  //! < <z*x> Mean value of z*y



    // ===========================================================
    // Line
    // ===========================================================

    Double_t fX1 = -1.;
    Double_t fY1 = -1.;
    Double_t fZ1 = -1.;
    Double_t fX2 = -1.;
    Double_t fY2 = -1.;
    Double_t fZ2 = -1.;
    Double_t fRMS = -1.;

   ODRFitter *fODRFitter = nullptr; //!

  public:

    STNLHit() { Clear(); };
    virtual ~STNLHit() {}
    virtual void Clear(Option_t * ="");


    // ===========================================================
    // Hit
    // ===========================================================

    void SetIsCluster (Bool_t val)   { fIsCluster = val; }
    void SetID        (Int_t val)    { fID = val; }
    void SetHitID     (Int_t val)    { fID = val; fIsCluster = false; }
    void SetClusterID (Int_t val)    { fID = val; fIsCluster = true; }
    void SetBarID     (Int_t val)    { fBarID = val; }
    void SetTOF       (Double_t val) { fTOF = val; }
    void SetBeta      (Double_t val) { fBeta = val; }
    void SetPosition  (TVector3 val) { fPosition = val; }

      Bool_t GetIsCluster() const { return fIsCluster; }
       Int_t GetID()        const { return fID; }
       Int_t GetHitID()     const { return fID; }
       Int_t GetClusterID() const { return fID; }
       Int_t GetBarID()     const { return fBarID; }
    Double_t GetTOF()       const { return fTOF; }
    Double_t GetBeta()      const { return fBeta; }
    TVector3 GetPosition()  const { return fPosition; }

    Double_t GetX() const { return fPosition.X(); }
    Double_t GetY() const { return fPosition.Y(); }
    Double_t GetZ() const { return fPosition.Z(); }

    void SetQ(Double_t q) { fQ = q; }
    void SetCharge(Double_t q) { fQ = q; }

    Double_t GetQ() const { return fQ; }
    Double_t GetCharge() const { return fQ; }

    // ===========================================================
    // Cluster
    // ===========================================================

    void AddHit(STNLHit *hit);
    void FitLine();
    
    vector<STNLHit*> *GetHitPtrs() { return &fHitPtrs; }
    vector<Int_t>    *GetHitIDs()  { return &fHitIDs;  }

    Int_t GetN() const { return fN; }
    Int_t GetNumHits() const { return fN; }
    Int_t GetClusterSize() const { return fN; }

    Double_t GetAXX() const { return fQ * (fEXX - fEX * fEX); } ///< W * SUM_i (<x>-x_i)^2 : diagonal compoent of matrix A
    Double_t GetAYY() const { return fQ * (fEYY - fEY * fEY); } ///< W * SUM_i (<y>-y_i)^2 : diagonal compoent of matrix A
    Double_t GetAZZ() const { return fQ * (fEZZ - fEZ * fEZ); } ///< W * SUM_i (<z>-z_i)^2 : diagonal compoent of matrix A
    Double_t GetAXY() const { return fQ * (fEXY - fEX * fEY); } ///< W * SUM_i (<x>-x_i)*(<y>-y_i) : off-diagonal compoent of matrix A
    Double_t GetAYZ() const { return fQ * (fEYZ - fEY * fEZ); } ///< W * SUM_i (<y>-y_i)*(<z>-z_i) : off-diagonal compoent of matrix A
    Double_t GetAZX() const { return fQ * (fEZX - fEZ * fEX); } ///< W * SUM_i (<z>-z_i)*(<x>-x_i) : off-diagonal compoent of matrix A


    // ===========================================================
    // Line
    // ===========================================================

    void SetRMS(Double_t rms) { fRMS = rms; }
    Double_t GetRMS() { return fRMS; }

    virtual TVector3 GetCenter() const { return .5*TVector3(fX1+fX2, fY1+fY2, fZ1+fZ2); }
    void SetLine(Double_t x1, Double_t y1, Double_t z1, Double_t x2, Double_t y2, Double_t z2);
    void SetLine(TVector3 pos1, TVector3 pos2);

    Double_t GetX1() const { return fX1; }
    Double_t GetY1() const { return fY1; }
    Double_t GetZ1() const { return fZ1; }
    Double_t GetX2() const { return fX2; }
    Double_t GetY2() const { return fY2; }
    Double_t GetZ2() const { return fZ2; }
    TVector3 GetPoint1() const { return TVector3(fX1, fY1, fZ1); }
    TVector3 GetPoint2() const { return TVector3(fX2, fY2, fZ2); }
    TVector3 Direction() const; ///< 1 to 2

    Double_t Length(Double_t x, Double_t y, Double_t z) const;
    Double_t Length(TVector3 position) const;
    Double_t Length() const;

    void ClosestPointOnLine(Double_t x, Double_t y, Double_t z,
        Double_t &x0, Double_t &y0, Double_t &z0) const;
    TVector3 ClosestPointOnLine(TVector3 pos) const;
    Double_t DistanceToLine(Double_t x, Double_t y, Double_t z) const;
    Double_t DistanceToLine(TVector3 pos) const;

    TArrow *DrawArrowXY(Double_t asize = 0.02) { return new TArrow(fX1, fY1, fX2, fY2, asize); }
    TArrow *DrawArrowYZ(Double_t asize = 0.02) { return new TArrow(fY1, fZ1, fY2, fZ2, asize); }
    TArrow *DrawArrowZY(Double_t asize = 0.02) { return new TArrow(fZ1, fY1, fZ2, fY2, asize); }
    TArrow *DrawArrowZX(Double_t asize = 0.02) { return new TArrow(fZ1, fX1, fZ2, fX2, asize); }
    TArrow *DrawArrowXZ(Double_t asize = 0.02) { return new TArrow(fX1, fZ1, fX2, fZ2, asize); }

    Double_t Residual(TVector3 point) const { return DistanceToLine(point); }



  ClassDef(STNLHit, 1);

};

class STNLHitSortZInv {
  public:
    Bool_t operator() (STNLHit* hit1, STNLHit* hit2)
    { return hit1 -> GetZ() < hit2 -> GetZ(); }
};


#endif
