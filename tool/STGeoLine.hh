#ifndef STGEOLINE_HH
#define STGEOLINE_HH

#include "TVector3.h"
#include "TArrow.h"

class STGeoLine
{
  public:
    STGeoLine();
    STGeoLine(Double_t x1, Double_t y1, Double_t z1, Double_t x2, Double_t y2, Double_t z2);
    STGeoLine(TVector3 pos1, TVector3 pos2);
    virtual ~STGeoLine() {}

    void SetLine(Double_t x1, Double_t y1, Double_t z1, Double_t x2, Double_t y2, Double_t z2);
    void SetLine(TVector3 pos1, TVector3 pos2);

    Double_t GetX1() const;
    Double_t GetY1() const;
    Double_t GetZ1() const;
    Double_t GetX2() const;
    Double_t GetY2() const;
    Double_t GetZ2() const;

    TVector3 GetPoint1() const;
    TVector3 GetPoint2() const;

    TVector3 Direction() const;

    Double_t Length(Double_t x, Double_t y, Double_t z) const;
    Double_t Length(TVector3 position) const;
    Double_t Length() const;

    void POCA(Double_t x, Double_t y, Double_t z, Double_t &x0, Double_t &y0, Double_t &z0) const;
    TVector3 POCA(TVector3 pos) const;

    TVector3 POCATo(TVector3 pos) const;

    Double_t DistanceToLine(Double_t x, Double_t y, Double_t z) const;
    Double_t DistanceToLine(TVector3 pos) const;

    TArrow *CreateTArrowXY();
    TArrow *CreateTArrowYZ();
    TArrow *CreateTArrowZY();
    TArrow *CreateTArrowZX();
    TArrow *CreateTArrowXZ();

  protected:
    Double_t fX1 = 0;
    Double_t fY1 = 0;
    Double_t fZ1 = 0;

    Double_t fX2 = 0;
    Double_t fY2 = 0;
    Double_t fZ2 = 0;


  ClassDef(STGeoLine, 1)
};

#endif
