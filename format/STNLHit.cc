#include <cmath>
#include "STNLHit.hh"

ClassImp(STNLHit);

void STNLHit::Clear(Option_t *)
{
  // ===========================================================
  // Hit
  // ===========================================================

  fIsCluster = false;
  fID = -1;
  fBarID = -1;
  fTOF = -1.;
  fBeta = -1.;
  fPosition = TVector3(-1,-1,-1); ///< local position

  fQ = 0.;  ///< (Sum of) charge


  // ===========================================================
  // Cluster
  // ===========================================================

  fHitPtrs.clear(); //!
  fHitIDs.clear(); ///<

  fN = 0;

  fEX  = 0;  //! < <x>   Mean value of x
  fEY  = 0;  //! < <y>   Mean value of y
  fEZ  = 0;  //! < <z>   Mean value of z
  fEXX = 0;  //! < <x*x> Mean value of x*x
  fEYY = 0;  //! < <y*y> Mean value of y*y
  fEZZ = 0;  //! < <z*z> Mean value of z*z
  fEXY = 0;  //! < <x*y> Mean value of x*y
  fEYZ = 0;  //! < <y*z> Mean value of y*z
  fEZX = 0;  //! < <z*x> Mean value of z*y



  // ===========================================================
  // Line
  // ===========================================================

  fX1 = -1.;
  fY1 = -1.;
  fZ1 = -1.;
  fX2 = -1.;
  fY2 = -1.;
  fZ2 = -1.;
  fRMS = -1.;

  //STODRFitter *fODRFitter = nullptr; //!
}

void STNLHit::AddHit(STNLHit *hit)
{
  ++fN;
  fIsCluster = true;
  fHitPtrs.push_back(hit);
  fHitIDs.push_back(hit->GetID());



  Double_t w = hit -> GetQ();
  Double_t x = hit -> GetX();
  Double_t y = hit -> GetY();
  Double_t z = hit -> GetZ();
  Double_t wsum = fQ + w;

  fEX  = (fQ*fEX + w*x) / wsum;
  fEY  = (fQ*fEY + w*y) / wsum;
  fEZ  = (fQ*fEZ + w*z) / wsum;
  fEXX = (fQ*fEXX + w*x*x) / wsum;
  fEYY = (fQ*fEYY + w*y*y) / wsum;
  fEZZ = (fQ*fEZZ + w*z*z) / wsum;
  fEXY = (fQ*fEXY + w*x*y) / wsum;
  fEYZ = (fQ*fEYZ + w*y*z) / wsum;
  fEZX = (fQ*fEZX + w*z*x) / wsum;

  fQ = wsum;



  if (fN > 1)
    FitLine();
}

void STNLHit::FitLine()
{
  if (fN < 2)
    return;

  if (fODRFitter == nullptr)
    fODRFitter = ODRFitter::GetFitter();

  else if (fN == 2) 
  {
    SetLine(fHitPtrs[0]->GetPosition(), fHitPtrs[1]->GetPosition());
    fRMS = 0.;
  }
  else if (fN > 2)
  {
    fODRFitter -> Reset();
    fODRFitter -> SetCentroid(fEX,fEY,fEZ);
    fODRFitter -> SetMatrixA(GetAXX(),GetAYY(),GetAZZ(),GetAXY(),GetAYZ(),GetAZX());
    fODRFitter -> SetWeightSum(fQ);
    fODRFitter -> SetNumPoints(fN);

    if (fODRFitter -> Solve() == false)
      return;

    fODRFitter -> ChooseEigenValue(0);

    SetLine(TVector3(fEX,fEY,fEZ),fODRFitter -> GetDirection());
    fRMS = fODRFitter -> GetRMSLine();
  }
}


void STNLHit::SetLine(Double_t x1, Double_t y1, Double_t z1, Double_t x2, Double_t y2, Double_t z2)
{
  fX1 = x1;
  fY1 = y1;
  fZ1 = z1;
  fX2 = x2;
  fY2 = y2;
  fZ2 = z2;

  if (fN > 2)
  {
    TVector3 pos_min;
    TVector3 pos_max;

    Double_t length_min = DBL_MAX;
    Double_t length_max = -DBL_MAX;

    for (auto hit : fHitPtrs)
    {
      auto pos = hit -> GetPosition();
      auto length = Length(pos);

      if (length < length_min) { length_min = length; pos_min = pos; }
      if (length > length_max) { length_max = length; pos_max = pos; }
    }

    pos_min = ClosestPointOnLine(pos_min);
    pos_max = ClosestPointOnLine(pos_max);

    fX1 = pos_min.X();
    fY1 = pos_min.Y();
    fZ1 = pos_min.Z();
    fX2 = pos_max.X();
    fY2 = pos_max.Y();
    fZ2 = pos_max.Z();
  }
}

void STNLHit::SetLine(TVector3 pos1, TVector3 pos2)
{
  SetLine(pos1.X() ,pos1.Y() ,pos1.Z() ,pos2.X() ,pos2.Y() ,pos2.Z());
}

TVector3 STNLHit::Direction() const
{
  auto v = TVector3(fX2-fX1, fY2-fY1, fZ2-fZ1);
  return v.Unit();
}

Double_t STNLHit::Length(Double_t x, Double_t y, Double_t z) const
{
  auto length = std::sqrt((fX1-x)*(fX1-x) + (fY1-y)*(fY1-y) + (fZ1-z)*(fZ1-z)); 
  auto direction = TVector3(fX1-x, fY1-y, fZ1-z).Dot(TVector3(fX1-fX2, fY1-fY2, fZ1-fZ2));
  if (direction > 0)
    direction = 1;
  else
    direction = -1;

  return direction * length;
}

Double_t STNLHit::Length(TVector3 position) const { return Length(position.X(), position.Y(), position.Z()); }
Double_t STNLHit::Length() const { return std::sqrt((fX1-fX2)*(fX1-fX2) + (fY1-fY2)*(fY1-fY2) + (fZ1-fZ2)*(fZ1-fZ2)); }

void STNLHit::ClosestPointOnLine(Double_t x, Double_t y, Double_t z, Double_t &x0, Double_t &y0, Double_t &z0) const
{
  Double_t xv = fX2 - fX1;
  Double_t yv = fY2 - fY1;
  Double_t zv = fZ2 - fZ1;

  Double_t norm = 1./std::sqrt(xv*xv + yv*yv + zv*zv);

  xv = norm*xv;
  yv = norm*yv;
  zv = norm*zv;

  Double_t xp = x - fX1;
  Double_t yp = y - fY1;
  Double_t zp = z - fZ1;

  Double_t l = xv*xp + yv*yp + zv*zp;

  x0 = fX1 + l*xv;
  y0 = fY1 + l*yv;
  z0 = fZ1 + l*zv;
}

TVector3 STNLHit::ClosestPointOnLine(TVector3 pos) const
{
  Double_t x0 = 0, y0 = 0, z0 = 0;
  ClosestPointOnLine(pos.X(), pos.Y(), pos.Z(), x0, y0, z0);

  return TVector3(x0, y0, z0);
}

Double_t STNLHit::DistanceToLine(Double_t x, Double_t y, Double_t z) const
{
  Double_t x0 = 0, y0 = 0, z0 = 0;

  ClosestPointOnLine(x, y, z, x0, y0, z0);

  return std::sqrt((x-x0)*(x-x0) + (y-y0)*(y-y0) + (z-z0)*(z-z0));
}

Double_t STNLHit::DistanceToLine(TVector3 pos) const
{
  Double_t x0 = 0, y0 = 0, z0 = 0;

  ClosestPointOnLine(pos.X(), pos.Y(), pos.Z(), x0, y0, z0);

  return std::sqrt((pos.X()-x0)*(pos.X()-x0) + (pos.Y()-y0)*(pos.Y()-y0) + (pos.Z()-z0)*(pos.Z()-z0));
}
