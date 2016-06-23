#include "STHelixTrack.hh"

#include <iostream>
using namespace std;

ClassImp(STHelixTrack)

STHelixTrack::STHelixTrack()
{
  Clear();
}

STHelixTrack::STHelixTrack(Int_t id)
{
  Clear();
  fTrackID = id;
}

void STHelixTrack::Clear(Option_t *option)
{
  fTrackID  = -999;
  fParentID = -999;

  fFitStatus = kNon;

  fXHelixCenter = -999;
  fZHelixCenter = -999;
  fHelixRadius  = -999;
  fYInitial     = -999;

  fChargeSum = 0;

  fXMean = 0;
  fYMean = 0;
  fZMean = 0;
  fXCov  = 0;
  fZCov  = 0;

  fRMSW = -999;
  fRMSH = -999;

  fAlphaHead = -999;
  fAlphaTail = -999;

  if (TString(option) == "C")
    DeleteHits();
  else {
    fMainHits.clear();
    fCandHits.clear();
  }
}

void STHelixTrack::Print(Option_t *option) const
{
  TString fitStat;
  if      (fFitStatus == STHelixTrack::kNon)   fitStat = "Not Set";
  else if (fFitStatus == STHelixTrack::kHelix) fitStat = "Helix";
  else if (fFitStatus == STHelixTrack::kLine)  fitStat = "Straight Line";
  TString center = "("+TString::Itoa(fXHelixCenter,10)+", x, "+TString::Itoa(fZHelixCenter,10)+")";

  cout << left << " STHelixTrack, units in [mm] [radian] [ADC]" << endl;
  cout << " - " << setw(13) << "Track ID"     << " : " << fTrackID << endl;
  cout << " - " << setw(13) << "Parent ID"    << " : " << fParentID << endl;
  cout << " - " << setw(13) << "Fit Status"   << " : " << fitStat << endl;
  cout << " - " << setw(13) << "Helix Center" << " : " << center << " [mm]" << endl;
  cout << " - " << setw(13) << "Helix Radius" << " : " << fHelixRadius << " [mm]" << endl;
  cout << " - " << setw(13) << "Dip Angle"    << " : " << DipAngle() << endl;
  cout << " - " << setw(13) << "Fit RMS-w/h"  << " : " << fRMSW << " / " << fRMSH << " [mm]" << endl;
  cout << " - " << setw(13) << "Charge"       << " : " << fChargeSum << " [ADC]" << endl;;
  cout << " - " << setw(13) << "Track Length" << " : " << TrackLength() << " [mm]" << endl;;
  //cout << " - " << setw(13) << "Track Length" << " : " << Continuity()*TrackLength() << " / " << TrackLength() << " [mm]" << endl;;
  //cout << " - " << setw(13) << "Simple dE/dx" << " : " << fChargeSum/Continuity()*TrackLength() << " [mm]" << endl;;
  cout << " - " << setw(13) << "Momentum"     << " : " << Momentum() << " [MeV]" << endl;;
}

void STHelixTrack::AddHit(STHit *hit)
{
  Double_t x = hit -> GetX();
  Double_t y = hit -> GetY();
  Double_t z = hit -> GetZ();
  Double_t w = hit -> GetCharge();

  Double_t W = fChargeSum + w;

  fXMean = (fChargeSum * fXMean + w * x) / W;
  fYMean = (fChargeSum * fYMean + w * y) / W;
  fZMean = (fChargeSum * fZMean + w * z) / W;

  if (fChargeSum != 0) 
  {
    fXCov = fChargeSum / W * fXCov + w * (fXMean - x) * (fXMean - x) / fChargeSum;
    fZCov = fChargeSum / W * fZCov + w * (fZMean - z) * (fZMean - z) / fChargeSum;
  }

  fChargeSum = W;

  fMainHits.push_back(hit);
  //fCandHits.push_back(hit);
}

void STHelixTrack::Remove(STHit *hit)
{
  Double_t x = hit -> GetX();
  Double_t y = hit -> GetY();
  Double_t z = hit -> GetZ();
  Double_t w = hit -> GetCharge();

  Double_t W = fChargeSum - w;

  fMainHits.push_back(hit);

  auto numHits = fMainHits.size();
  for (auto iHit = 0; iHit < numHits; iHit++) {
    if (fMainHits[iHit] == hit) {
      fMainHits.erase(fMainHits.begin()+iHit);
      break;
    }
  }

  fXMean = (fChargeSum * fXMean - w * x) / W;
  fYMean = (fChargeSum * fYMean - w * y) / W;
  fZMean = (fChargeSum * fZMean - w * z) / W;

  /*
  if (fChargeSum != 0) 
  {
    fXCov = fChargeSum / W * fXCov - w * (fXMean - x) * (fXMean - x) / fChargeSum;
    fZCov = fChargeSum / W * fZCov - w * (fZMean - z) * (fZMean - z) / fChargeSum;
  }
  */

  fChargeSum = W;
}

void STHelixTrack::DeleteHits()
{
  for (auto hit : fMainHits)
    delete hit;

  fMainHits.clear();

  for (auto hit : fCandHits)
    delete hit;

  fCandHits.clear();
}

void STHelixTrack::SortHits(bool increasing)
{
  if (increasing) {
    auto sorting = STHitSortByIncreasingLength(this);
    sort(fMainHits.begin(), fMainHits.end(), sorting);
  } else {
    auto sorting = STHitSortByDecreasingLength(this);
    sort(fMainHits.begin(), fMainHits.end(), sorting);
  }
}

void STHelixTrack::SetTrackID(Int_t idx)                  { fTrackID = idx; }
void STHelixTrack::SetParentID(Int_t idx)                 { fParentID = idx; }
void STHelixTrack::SetFitStatus(STFitStatus value)        { fFitStatus = value; }
void STHelixTrack::SetIsHelix()                           { fFitStatus = STHelixTrack::kHelix; }
void STHelixTrack::SetIsStraightLine()                    { fFitStatus = STHelixTrack::kLine; }
void STHelixTrack::SetHelixCenter(Double_t x, Double_t z) { fXHelixCenter = x; fZHelixCenter = z; }
void STHelixTrack::SetHelixRadius(Double_t r)             { fHelixRadius = r; }
void STHelixTrack::SetYInitial(Double_t y)                { fYInitial = y; }
void STHelixTrack::SetAlphaSlope(Double_t s)              { fAlphaSlope = s; }
void STHelixTrack::SetRMSW(Double_t rms)                  { fRMSW = rms; }
void STHelixTrack::SetRMSH(Double_t rms)                  { fRMSH = rms; }
void STHelixTrack::SetAlphaHead(Double_t alpha)           { fAlphaHead = alpha; }
void STHelixTrack::SetAlphaTail(Double_t alpha)           { fAlphaTail = alpha; }

Int_t STHelixTrack::GetTrackID()  const { return fTrackID; }
Int_t STHelixTrack::GetParentID() const { return fParentID; }

STHelixTrack::STFitStatus STHelixTrack::GetFitStatus() const { return fFitStatus; }

Double_t STHelixTrack::GetHelixCenterX() const { return fXHelixCenter; }
Double_t STHelixTrack::GetHelixCenterZ() const { return fZHelixCenter; }
Double_t STHelixTrack::GetHelixRadius()  const { return fHelixRadius; }
Double_t STHelixTrack::GetYInitial()     const { return fYInitial; }
Double_t STHelixTrack::GetAlphaSlope()   const { return fAlphaSlope; }

Double_t STHelixTrack::DipAngle() const
{
  if (fHelixRadius <= 0)
    return -999;

  return TMath::ATan(fAlphaSlope/fHelixRadius);
}


void STHelixTrack::GetHelixParameters(Double_t &xCenter, 
    Double_t &zCenter, 
    Double_t &radius, 
    Double_t &dipAngle,
    Double_t &yInitial,
    Double_t &alphaSlope) const
{
  if (fFitStatus == STHelixTrack::kHelix)
  {
    xCenter    = fXHelixCenter;
    zCenter    = fZHelixCenter;
    radius     = fHelixRadius;
    dipAngle   = DipAngle();
    yInitial   = fYInitial;
    alphaSlope = fAlphaSlope;
  }
  else
  {
    xCenter    = -999;
    zCenter    = -999;
    radius     = -999;
    dipAngle   = -999;
    yInitial   = -999;
    alphaSlope = -999;
  }
}

Double_t STHelixTrack::GetChargeSum()  const { return fChargeSum; }
Double_t STHelixTrack::GetXMean()      const { return fXMean; }
Double_t STHelixTrack::GetYMean()      const { return fYMean; }
Double_t STHelixTrack::GetZMean()      const { return fZMean; }
Double_t STHelixTrack::GetXCov()       const { return fXCov; }
Double_t STHelixTrack::GetZCov()       const { return fZCov; }
Double_t STHelixTrack::GetRMSW()       const { return fRMSW; }
Double_t STHelixTrack::GetRMSH()       const { return fRMSH; }
Double_t STHelixTrack::GetAlphaHead()  const { return fAlphaHead; }
Double_t STHelixTrack::GetAlphaTail()  const { return fAlphaTail; }

Int_t STHelixTrack::GetNumHits() const { return fMainHits.size(); }
STHit *STHelixTrack::GetHit(Int_t idx) const { return fMainHits.at(idx); }
std::vector<STHit *> *STHelixTrack::GetHitArray() { return &fMainHits; }

Int_t STHelixTrack::GetNumCandHits() const { return fCandHits.size(); }
std::vector<STHit *> *STHelixTrack::GetCandHitArray() { return &fCandHits; }

Double_t 
STHelixTrack::DistCircle(TVector3 pointGiven) const
{
  Double_t dx = pointGiven.X() - fXHelixCenter;
  Double_t dz = pointGiven.Z() - fZHelixCenter;
  return sqrt(dx*dx + dz*dz) - fHelixRadius;
}

Int_t STHelixTrack::Charge()   const { return DipAngle()*fAlphaSlope > 0 ? 1 : -1; }
Int_t STHelixTrack::Helicity() const { return fAlphaSlope > 0 ? 1 : -1; }

TVector3 STHelixTrack::PositionAtHead() const { return PositionByAlpha(fAlphaHead); }
TVector3 STHelixTrack::PositionAtTail() const { return PositionByAlpha(fAlphaTail); }

Double_t STHelixTrack::Momentum(Double_t B) const
{
  if (fFitStatus != STHelixTrack::kHelix)
    return -1;

  Double_t cosDip = TMath::Cos(DipAngle());
  if (cosDip < 1E-2)
    return TMath::Abs(fHelixRadius/1.E-2 * 0.3 * B);

  return TMath::Abs(fHelixRadius/cosDip * 0.3 * B);
}

Double_t STHelixTrack::TrackLength() const { 
  return std::abs(GetAlphaHead() - GetAlphaTail()) * fHelixRadius / TMath::Cos(DipAngle());
}
Double_t STHelixTrack::LengthInPeriod()  const { 
  return 2*TMath::Pi()*fHelixRadius/TMath::Cos(DipAngle());
}
Double_t STHelixTrack::YLengthInPeriod() const { 
  return 2*TMath::Pi()*fAlphaSlope;
}
Double_t STHelixTrack::LengthByAlpha(Double_t alpha) const { 
  return alpha*fHelixRadius/TMath::Cos(DipAngle());
}
Double_t STHelixTrack::AlphaByLength(Double_t length) const { 
  return length*TMath::Cos(DipAngle())/fHelixRadius;
}
TVector3 STHelixTrack::PositionByAlpha(Double_t alpha) const {

  return TVector3(fHelixRadius*TMath::Cos(alpha)+fXHelixCenter, alpha*fAlphaSlope+fYInitial, fHelixRadius*TMath::Sin(alpha)+fZHelixCenter); 
}

TVector3 STHelixTrack::Direction(Double_t alpha) const
{
  Double_t alphaTemp = alpha;
  Double_t ylength = YLengthInPeriod()/4.;

  Double_t alphaMid = (fAlphaHead + fAlphaTail) * 0.5;
  if (alpha > alphaMid) 
    alphaTemp += TMath::Pi()/2.;
  else {
    alphaTemp -= TMath::Pi()/2.;
    ylength *= -1;
  }

  TVector3 center(fXHelixCenter, 0, fZHelixCenter);
  TVector3 direction = PositionByAlpha(alphaTemp) - center;

  direction.SetY(0);
  direction.SetMag(0.5*TMath::Pi()*fHelixRadius);
  direction.SetY(ylength);
  direction = direction.Unit();

  return direction;
}

Double_t 
STHelixTrack::ExtrapolateToAlpha(Double_t alpha) const 
{
  return alpha * fHelixRadius / TMath::Cos(DipAngle());
}

Double_t
STHelixTrack::ExtrapolateToAlpha(Double_t alpha, TVector3 &pointOnHelix) const
{
  pointOnHelix.SetXYZ(fHelixRadius*TMath::Cos(alpha)+fXHelixCenter, alpha*fAlphaSlope + fYInitial, fHelixRadius*TMath::Sin(alpha)+fZHelixCenter);
  Double_t length = alpha * fHelixRadius / TMath::Cos(DipAngle());

  return length;
}

Double_t
STHelixTrack::ExtrapolateToPointAlpha(TVector3 pointGiven, TVector3 &pointOnHelix, Double_t &alpha) const
// TODO
{
  Double_t alpha0 = TMath::ATan2(pointGiven.Z()-fZHelixCenter, pointGiven.X()-fXHelixCenter);

  TVector3 point0(fHelixRadius*TMath::Cos(alpha0)+fXHelixCenter, alpha0*fAlphaSlope+fYInitial, fHelixRadius*TMath::Sin(alpha0)+fZHelixCenter);
  Double_t y0 = std::abs(point0.Y() - pointGiven.Y());

  Double_t y1; 
  Double_t alpha1 = alpha0;
  TVector3 point1 = point0;

  while(1) 
  {
    alpha1 = alpha1 + 2*TMath::Pi();
    point1.SetY(point1.Y() + 2*TMath::Pi()*fAlphaSlope);
    y1 = std::abs(point1.Y() - pointGiven.Y());

    if (y0 <= y1)
      break;
    else {
      alpha0 = alpha1;
      point0 = point1;
      y0 = y1;
    }
  }

  while(1) 
  {
    alpha1 = alpha1 - 2*TMath::Pi();
    point1.SetY(point1.Y() - 2*TMath::Pi()*fAlphaSlope);
    y1 = std::abs(point1.Y() - pointGiven.Y());

    if (y0 <= y1)
      break;
    else {
      alpha0 = alpha1;
      point0 = point1;
      y0 = y1;
    }
  }

  pointOnHelix = point0;
  alpha = alpha0;
  Double_t length = alpha0 * fHelixRadius / TMath::Cos(DipAngle());

  return length;
}

Double_t
STHelixTrack::ExtrapolateToPointY(TVector3 pointGiven, TVector3 &pointOnHelix, Double_t &alpha) const
{
  alpha = (pointGiven.Y() - fYInitial)/fAlphaSlope;
  pointOnHelix.SetXYZ(fHelixRadius*TMath::Cos(alpha)+fXHelixCenter, pointGiven.Y(), fHelixRadius*TMath::Sin(alpha)+fZHelixCenter);

  Double_t length = alpha * fHelixRadius / TMath::Cos(DipAngle());
  return length;
}

bool
STHelixTrack::CheckExtrapolateToX(Double_t x) const
{
  Double_t xRef = fXHelixCenter - x;
  Double_t mult = (xRef + fHelixRadius) * (xRef - fHelixRadius);
  if (mult > 0)
    return false;
  return true;
}

bool
STHelixTrack::CheckExtrapolateToZ(Double_t z) const
{
  Double_t zRef = fZHelixCenter - z;
  Double_t mult = (zRef + fHelixRadius) * (zRef - fHelixRadius);
  if (mult > 0)
    return false;
  return true;
}

bool
STHelixTrack::ExtrapolateToX(Double_t x,
    TVector3 &pointOnHelix1, Double_t &alpha1,
    TVector3 &pointOnHelix2, Double_t &alpha2) const
{
  if (CheckExtrapolateToX(x) == false)
    return false;

  Double_t zOff = sqrt(fHelixRadius * fHelixRadius - (x - fXHelixCenter) * (x - fXHelixCenter));
  Double_t z1 = fZHelixCenter + zOff;
  Double_t z2 = fZHelixCenter - zOff;

  alpha1 = TMath::ATan2(z1-fZHelixCenter, x-fXHelixCenter);
  Double_t alpha1Temp = alpha1;
  Double_t d1Cand = std::abs(alpha1Temp-fAlphaHead);
  Double_t d1Temp = d1Cand;

  while (1) {
    alpha1Temp = alpha1 + 2*TMath::Pi();
    d1Temp = std::abs(alpha1Temp-fAlphaHead);
    if (d1Temp >= d1Cand)
      break;
    else {
      alpha1 = alpha1Temp;
      d1Cand = d1Temp;
    }
  }
  while (1) {
    alpha1Temp = alpha1 - 2*TMath::Pi();
    d1Temp = std::abs(alpha1Temp-fAlphaHead);
    if (d1Temp >= d1Cand)
      break;
    else {
      alpha1 = alpha1Temp;
      d1Cand = d1Temp;
    }
  }
  pointOnHelix1 = PositionByAlpha(alpha1);

  alpha2 = TMath::ATan2(z2-fZHelixCenter, x-fXHelixCenter);
  Double_t alpha2Temp = alpha2;
  Double_t d2Cand = std::abs(alpha2Temp-fAlphaTail);
  Double_t d2Temp = d2Cand;

  while (1) {
    alpha2Temp = alpha2 + 2*TMath::Pi();
    d2Temp = std::abs(alpha2Temp-fAlphaTail);
    if (d2Temp >= d2Cand)
      break;
    else {
      alpha2 = alpha2Temp;
      d2Cand = d2Temp;
    }
  }
  while (1) {
    alpha2Temp = alpha2 - 2*TMath::Pi();
    d2Temp = std::abs(alpha2Temp-fAlphaTail);
    if (d2Temp >= d2Cand)
      break;
    else {
      alpha2 = alpha2Temp;
      d2Cand = d2Temp;
    }
  }
  pointOnHelix2 = PositionByAlpha(alpha2);

  return true;
}

bool
STHelixTrack::ExtrapolateToZ(Double_t z,
    TVector3 &pointOnHelix1, Double_t &alpha1,
    TVector3 &pointOnHelix2, Double_t &alpha2) const
{
  if (CheckExtrapolateToZ(z) == false)
    return false;

  Double_t xOff = sqrt(fHelixRadius * fHelixRadius - (z - fZHelixCenter) * (z - fZHelixCenter));
  Double_t x1 = fXHelixCenter + xOff;
  Double_t x2 = fXHelixCenter - xOff;

  alpha1 = TMath::ATan2(z-fZHelixCenter, x1-fXHelixCenter);
  Double_t alpha1Temp = alpha1;
  Double_t d1Cand = std::abs(alpha1Temp-fAlphaHead);
  Double_t d1Temp = d1Cand;

  while (1) {
    alpha1Temp = alpha1 + 2*TMath::Pi();
    d1Temp = std::abs(alpha1Temp-fAlphaHead);
    if (d1Temp >= d1Cand)
      break;
    else {
      alpha1 = alpha1Temp;
      d1Cand = d1Temp;
    }
  }
  while (1) {
    alpha1Temp = alpha1 - 2*TMath::Pi();
    d1Temp = std::abs(alpha1Temp-fAlphaHead);
    if (d1Temp >= d1Cand)
      break;
    else {
      alpha1 = alpha1Temp;
      d1Cand = d1Temp;
    }
  }
  pointOnHelix1 = PositionByAlpha(alpha1);

  alpha2 = TMath::ATan2(z-fZHelixCenter, x2-fXHelixCenter);
  Double_t alpha2Temp = alpha2;
  Double_t d2Cand = std::abs(alpha2Temp-fAlphaTail);
  Double_t d2Temp = d2Cand;

  while (1) {
    alpha2Temp = alpha2 + 2*TMath::Pi();
    d2Temp = std::abs(alpha2Temp-fAlphaTail);
    if (d2Temp >= d2Cand)
      break;
    else {
      alpha2 = alpha2Temp;
      d2Cand = d2Temp;
    }
  }
  while (1) {
    alpha2Temp = alpha2 - 2*TMath::Pi();
    d2Temp = std::abs(alpha2Temp-fAlphaTail);
    if (d2Temp >= d2Cand)
      break;
    else {
      alpha2 = alpha2Temp;
      d2Cand = d2Temp;
    }
  }
  pointOnHelix2 = PositionByAlpha(alpha2);

  return true;
}

bool
STHelixTrack::ExtrapolateToZ(Double_t z, TVector3 &pointOnHelix) const
{
  TVector3 position1, position2;
  Double_t alpha1, alpha2;

  if (ExtrapolateToZ(z, position1, alpha1, position2, alpha2) == false)
    return false;

  Double_t alphaMid = (fAlphaHead + fAlphaTail)/2;

  if (std::abs(alpha1 - alphaMid) < std::abs(alpha2 - alphaMid))
    pointOnHelix = position1;
  else
    pointOnHelix = position2;

  return true;
}

TVector3 
STHelixTrack::ExtrapolateHead(Double_t length) const
{
  Double_t alpha = fAlphaHead;
  Double_t dAlpha = std::abs(AlphaByLength(length));

  if (fAlphaHead > fAlphaTail)
    alpha += dAlpha;
  else
    alpha -= dAlpha;

  return PositionByAlpha(alpha);
}

TVector3 
STHelixTrack::ExtrapolateTail(Double_t length) const
{
  Double_t alpha = fAlphaTail;
  Double_t dAlpha = std::abs(AlphaByLength(length));

  if (fAlphaTail > fAlphaHead)
    alpha += dAlpha;
  else
    alpha -= dAlpha;

  return PositionByAlpha(alpha);
}

TVector3 
STHelixTrack::InterpolateByRatio(Double_t r) const
{
  TVector3 q;
  ExtrapolateToAlpha(r*fAlphaHead+(1-r)*fAlphaTail, q);
  return q;
}

TVector3 
STHelixTrack::InterpolateByLength(Double_t length) const
{
  return InterpolateByRatio(length/TrackLength());
}

TVector3 
STHelixTrack::Map(TVector3 p) const
{
  TVector3 q, m;
  ExtrapolateByMap(p, q, m);

  return m;
}

Double_t 
STHelixTrack::ExtrapolateByMap(TVector3 p, TVector3 &q, TVector3 &m) const
{
  Double_t lHead = ExtrapolateToAlpha(fAlphaHead);
  Double_t lTail = ExtrapolateToAlpha(fAlphaTail);
  Double_t lOff = lHead;
  if (lHead > lTail)
    lOff = lTail;

  Double_t alpha;
  Double_t length = ExtrapolateToPointAlpha(p, q, alpha);
  Double_t r = DistCircle(p);
  Double_t y = p.Y() - q.Y();

  m = TVector3(r, y/TMath::Cos(DipAngle()), length+y*TMath::Sin(DipAngle()) - lOff);

  return alpha * fHelixRadius / TMath::Cos(DipAngle()); 
}

Double_t 
STHelixTrack::Continuity(Double_t &totalLength, Double_t &continuousLength)
{
  auto numHits = fMainHits.size();
  if (numHits < 2) 
    return -1;

  SortHits();

  Double_t total = 0;
  Double_t continuous = 0;
  TVector3 before = Map(fMainHits[0]->GetPosition());

  for (auto iHit = 1; iHit < numHits; iHit++) 
  {
    TVector3 current = Map(fMainHits[iHit]->GetPosition());
    auto length = std::abs(current.Z()-before.Z());

    total += length;
    if (length < 20)
      continuous += length;

    before = current;
  }

  totalLength = total;
  continuousLength = continuous;

  return continuous/total;
}

Double_t 
STHelixTrack::Continuity()
{
  auto numHits = fMainHits.size();
  if (numHits < 2) 
    return -1;

  SortHits();

  Double_t total = 0;
  Double_t continuous = 0;
  TVector3 before = Map(fMainHits[0]->GetPosition());

  for (auto iHit = 1; iHit < numHits; iHit++) 
  {
    TVector3 current = Map(fMainHits[iHit]->GetPosition());
    auto length = std::abs(current.Z()-before.Z());

    total += length;
    if (length < 20)
      continuous += length;

    before = current;
  }

  return continuous/total;
}
