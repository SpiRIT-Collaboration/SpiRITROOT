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
  fIsEmbed = false;
  fFitStatus = kBad;

  fXHelixCenter = -999;
  fZHelixCenter = -999;
  fHelixRadius  = -999;
  fYInitial     = -999;
  fAlphaSlope   = -999;

  fChargeSum = 0;

  fExpectationX = 0;
  fExpectationY = 0;
  fExpectationZ = 0;
  fExpectationXX = 0;
  fExpectationYY = 0;
  fExpectationZZ = 0;
  fExpectationXY = 0;
  fExpectationYZ = 0;
  fExpectationZX = 0;

  fRMSW = -999;
  fRMSH = -999;

  fAlphaHead = -999;
  fAlphaTail = -999;

  fIsPositiveChargeParticle = true;

  if (TString(option) == "C")
    DeleteHits();
  else {
    fMainHits.clear();
    fCandHits.clear();
    fHitClusters.clear();
  }

  fMainHitIDs.clear();
  fClusterIDs.clear();

  fGenfitID  = -999;
  fGenfitMomentum = -999;
}

void STHelixTrack::Print(Option_t *option) const
{
  TString center = "("+TString::Itoa(fXHelixCenter,10)+", x, "+TString::Itoa(fZHelixCenter,10)+")";

  cout << left << " STHelixTrack, units in [mm] [radian] [ADC]" << endl;
  cout << " - " << setw(13) << "Track ID"     << " : " << fTrackID << endl;
  cout << " - " << setw(13) << "Parent ID"    << " : " << fParentID << endl;
  cout << " - " << setw(13) << "Fit Status"   << " : " << GetFitStatusString() << endl;

  if (fFitStatus != STHelixTrack::kHelix && fFitStatus != STHelixTrack::kGenfitTrack)
    return;

  cout << " - " << setw(13) << "Helix Center" << " : " << center << " [mm]" << endl;
  cout << " - " << setw(13) << "Helix Radius" << " : " << fHelixRadius << " [mm]" << endl;
  cout << " - " << setw(13) << "Dip Angle"    << " : " << DipAngle() << endl;
  cout << " - " << setw(13) << "Fit RMS-w/h"  << " : " << fRMSW << " / " << fRMSH << " [mm]" << endl;
  cout << " - " << setw(13) << "Charge"       << " : " << fChargeSum << " [ADC]" << endl;;
  cout << " - " << setw(13) << "Track Length" << " : " << TrackLength() << " [mm]" << endl;;
  cout << " - " << setw(13) << "Momentum"     << " : " << Momentum() << " [MeV]" << endl;;

  if (fFitStatus == STHelixTrack::kGenfitTrack) {
    cout << " - " << setw(13) << "GF-Momentum"  << " : " << fGenfitMomentum << " [MeV]" << endl;;
    cout << " - " << setw(13) << "dEdx (70 %)"  << " : " << GetdEdxWithCut(0, 0.7) << " [ADC/mm]" << endl;;
  }
}

void STHelixTrack::AddHit(STHit *hit)
{
  Double_t x = hit -> GetX();
  Double_t y = hit -> GetY();
  Double_t z = hit -> GetZ();
  Double_t w = hit -> GetCharge();

  Double_t W = fChargeSum + w;

  fExpectationX = (fChargeSum * fExpectationX + w * x) / W;
  fExpectationY = (fChargeSum * fExpectationY + w * y) / W;
  fExpectationZ = (fChargeSum * fExpectationZ + w * z) / W;

  fExpectationXX = (fChargeSum * fExpectationXX + w * x * x) / W;
  fExpectationYY = (fChargeSum * fExpectationYY + w * y * y) / W;
  fExpectationZZ = (fChargeSum * fExpectationZZ + w * z * z) / W;

  fExpectationXY = (fChargeSum * fExpectationXY + w * x * y) / W;
  fExpectationYZ = (fChargeSum * fExpectationYZ + w * y * z) / W;
  fExpectationZX = (fChargeSum * fExpectationZX + w * z * x) / W;

  fChargeSum = W;

  fMainHits.push_back(hit);
}

void STHelixTrack::Remove(STHit *hit)
{
  Double_t x = hit -> GetX();
  Double_t y = hit -> GetY();
  Double_t z = hit -> GetZ();
  Double_t w = hit -> GetCharge();

  Double_t W = fChargeSum - w;

  auto numHits = fMainHits.size();
  for (auto iHit = 0; iHit < numHits; iHit++) {
    if (fMainHits[iHit] == hit) {
      fMainHits.erase(fMainHits.begin()+iHit);
      break;
    }
  }

  fExpectationX = (fChargeSum * fExpectationX - w * x) / W;
  fExpectationY = (fChargeSum * fExpectationY - w * y) / W;
  fExpectationZ = (fChargeSum * fExpectationZ - w * z) / W;

  fExpectationXX = (fChargeSum * fExpectationXX - w * x * x) / W;
  fExpectationYY = (fChargeSum * fExpectationYY - w * y * y) / W;
  fExpectationZZ = (fChargeSum * fExpectationZZ - w * z * z) / W;

  fExpectationXY = (fChargeSum * fExpectationXY - w * x * y) / W;
  fExpectationYZ = (fChargeSum * fExpectationYZ - w * y * z) / W;
  fExpectationZX = (fChargeSum * fExpectationZX - w * z * x) / W;

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

  for (auto cluster : fHitClusters)
    delete cluster;

  fHitClusters.clear();
}

void STHelixTrack::SortHits(bool increasing)
{
  if (TMath::Abs(YLengthInPeriod()) < 50) {
    auto sorting = STHitByDistanceTo(fVertexPosition);
    sort(fMainHits.begin(), fMainHits.end(), sorting);
  } else if (increasing) {
    auto sorting = STHitSortByIncreasingLength(this);
    sort(fMainHits.begin(), fMainHits.end(), sorting);
  } else {
    auto sorting = STHitSortByDecreasingLength(this);
    sort(fMainHits.begin(), fMainHits.end(), sorting);
  }
}

void STHelixTrack::SortClusters(bool increasing)
{
  if (TMath::Abs(YLengthInPeriod()) < 50) {
    auto sorting = STHitByDistanceTo(fVertexPosition);
    sort(fMainHits.begin(), fMainHits.end(), sorting);
  } else if (increasing) {
    auto sorting = STHitSortByIncreasingLength(this);
    sort(fHitClusters.begin(), fHitClusters.end(), sorting);
  } else {
    auto sorting = STHitSortByDecreasingLength(this);
    sort(fHitClusters.begin(), fHitClusters.end(), sorting);
  }
}

void STHelixTrack::SortHitsByTimeOrder() { SortHits(fIsPositiveChargeParticle); }

void STHelixTrack::SortClustersByTimeOrder() { SortHits(fIsPositiveChargeParticle); }

void STHelixTrack::AddHitCluster(STHitCluster *cluster)
{
  fHitClusters.push_back(cluster);
}

void STHelixTrack::FinalizeHits()
{
  if (TMath::Abs(YLengthInPeriod()) < 50) {
    for (auto hit : fMainHits) {
      fMainHitIDs.push_back(hit->GetHitID());
      hit -> SetTrackID(fTrackID);
      hit -> SetS((hit->GetPosition()-fVertexPosition).Mag());
    }
  }
  else if (fIsPositiveChargeParticle) {
    for (auto hit : fMainHits) {
      fMainHitIDs.push_back(hit->GetHitID());
      hit -> SetTrackID(fTrackID);
      hit -> SetS(Map(hit->GetPosition()).Z());
    }
  }
  else {
    for (auto hit : fMainHits) {
      fMainHitIDs.push_back(hit->GetHitID());
      hit -> SetTrackID(fTrackID);
      hit -> SetS(-Map(hit->GetPosition()).Z());
    }
  }
}

void STHelixTrack::FinalizeClusters()
{
  for (auto cluster : fHitClusters)
  {
    cluster -> SetTrackID(fTrackID);
    cluster -> ApplyCovLowLimit();
    if (cluster -> IsStable())
    {
      fClusterIDs.push_back(cluster->GetClusterID());
      fdEdxArray.push_back(cluster->GetCharge()/cluster->GetLength());
      if(cluster-> IsEmbed()==true)
	      fIsEmbed = true;
    }

    TVector3 dir = Direction(AlphaAtPosition(cluster->GetPosition()));
    auto chi = TMath::ATan2(abs(dir.X()),abs(dir.Z()));
    cluster -> SetChi(chi);
    cluster -> SetLambda(DipAngle());
  }

  auto maxx = 0.;
  auto maxy = 0.;
  auto maxz = 0.;

  for (auto cluster : fHitClusters)
  {
    auto cov = cluster -> GetCovMatrix();
    auto charge = cluster -> GetCharge();
    auto cx = abs(cov(0,0)/charge); if (cx > maxx) maxx = cx;
    auto cy = abs(cov(1,1)/charge); if (cy > maxy) maxy = cy;
    auto cz = abs(cov(2,2)/charge); if (cz > maxz) maxz = cz;
  }

  for (auto cluster : fHitClusters)
    cluster -> SetDFromCovForGenfit(maxx,maxy,maxz);
}


void STHelixTrack::SetTrackID(Int_t idx)    { fTrackID = idx; }
void STHelixTrack::SetGenfitID(Int_t idx)   { fGenfitID = idx; }
void STHelixTrack::SetParentID(Int_t idx)   { fParentID = idx; }

void STHelixTrack::SetFitStatus(STFitStatus value)  { fFitStatus = value; }
void STHelixTrack::SetIsEmbed(bool val){   fIsEmbed = val; }
void STHelixTrack::SetIsBad()          { fFitStatus = STHelixTrack::kBad; }
void STHelixTrack::SetIsLine()         { fFitStatus = STHelixTrack::kLine; }
void STHelixTrack::SetIsPlane()        { fFitStatus = STHelixTrack::kPlane; }
void STHelixTrack::SetIsHelix()        { fFitStatus = STHelixTrack::kHelix; }
void STHelixTrack::SetIsGenfitTrack()  { fFitStatus = STHelixTrack::kGenfitTrack; }

void STHelixTrack::SetLineDirection(TVector3 dir)
{
  fXHelixCenter = dir.X();
  fZHelixCenter = dir.Y();
  fHelixRadius = dir.Z();
}

void STHelixTrack::SetPlaneNormal(TVector3 norm)
{
  fXHelixCenter = norm.X();
  fZHelixCenter = norm.Y();
  fHelixRadius = norm.Z();
}

void STHelixTrack::SetHelixCenter(Double_t x, Double_t z) { fXHelixCenter = x; fZHelixCenter = z; }
void STHelixTrack::SetHelixRadius(Double_t r)    { fHelixRadius = r; }
void STHelixTrack::SetYInitial(Double_t y)       { fYInitial = y; }
void STHelixTrack::SetAlphaSlope(Double_t s)     { fAlphaSlope = s; }
void STHelixTrack::SetRMSW(Double_t rms)         { fRMSW = rms; }
void STHelixTrack::SetRMSH(Double_t rms)         { fRMSH = rms; }
void STHelixTrack::SetAlphaHead(Double_t alpha)  { fAlphaHead = alpha; }
void STHelixTrack::SetAlphaTail(Double_t alpha)  { fAlphaTail = alpha; }

void STHelixTrack::DetermineParticleCharge(TVector3 vertex)
{
  fVertexPosition = vertex;

  Double_t lHead = ExtrapolateToAlpha(fAlphaHead);
  Double_t lTail = ExtrapolateToAlpha(fAlphaTail);

  TVector3 q;
  Double_t alpha;
  Double_t lVertex = ExtrapolateToPointAlpha(vertex, q, alpha);

  if (std::abs(lVertex-lTail) > std::abs(lVertex - lHead))
    fIsPositiveChargeParticle = true;
  else
    fIsPositiveChargeParticle = false;
}

void STHelixTrack::SetIsPositiveChargeParticle(Bool_t val)  { fIsPositiveChargeParticle = val; }

void STHelixTrack::SetGenfitMomentum(Double_t p) { fGenfitMomentum = p; }

Int_t STHelixTrack::GetTrackID()  const  { return fTrackID; }
Int_t STHelixTrack::GetGenfitID() const  { return fGenfitID; }
Int_t STHelixTrack::GetParentID() const  { return fParentID; }

STHelixTrack::STFitStatus STHelixTrack::GetFitStatus() const { return fFitStatus; }

TString STHelixTrack::GetFitStatusString() const
{
  TString fitStat;

  if      (fFitStatus == STHelixTrack::kBad) fitStat = "Bad";
  else if (fFitStatus == STHelixTrack::kLine) fitStat = "Line";
  else if (fFitStatus == STHelixTrack::kPlane) fitStat = "Plane";
  else if (fFitStatus == STHelixTrack::kHelix) fitStat = "Helix";
  else if (fFitStatus == STHelixTrack::kGenfitTrack) fitStat = "Genfit";

  return fitStat;
}

bool STHelixTrack::IsEmbed() const        { return fIsEmbed;}
bool STHelixTrack::IsBad() const          { return fFitStatus == kBad   ? true : false; }
bool STHelixTrack::IsLine()  const        { return fFitStatus == kLine  ? true : false; }
bool STHelixTrack::IsPlane() const        { return fFitStatus == kPlane ? true : false; }
bool STHelixTrack::IsHelix() const        { return fFitStatus == kHelix ? true : false; }
bool STHelixTrack::IsGenfitTrack() const  { return fFitStatus == kGenfitTrack ? true : false; }

Double_t STHelixTrack::GetHelixCenterX() const { return fXHelixCenter; }
Double_t STHelixTrack::GetHelixCenterZ() const { return fZHelixCenter; }
Double_t STHelixTrack::GetHelixRadius()  const { return fHelixRadius; }
Double_t STHelixTrack::GetYInitial()     const { return fYInitial; }
Double_t STHelixTrack::GetAlphaSlope()   const { return fAlphaSlope; }

TVector3 STHelixTrack::GetLineDirection() const { return TVector3(fXHelixCenter, fZHelixCenter, fHelixRadius); }
TVector3 STHelixTrack::GetPlaneNormal()   const { return TVector3(fXHelixCenter, fZHelixCenter, fHelixRadius); }

TVector3 STHelixTrack::PerpLine(TVector3 p) const
{
  TVector3 mean = GetMean();
  TVector3 dir = GetLineDirection();

  TVector3 pMinusMean = p - mean;
  TVector3 pMinusMeanUnit = pMinusMean.Unit();
  Double_t cosine = pMinusMeanUnit.Dot(dir);
  dir.SetMag(pMinusMean.Mag()*cosine);

  return dir - pMinusMean;
}

TVector3 STHelixTrack::PerpPlane(TVector3 p) const
{
  TVector3 normal = GetPlaneNormal();
  TVector3 mean = GetMean();

  Double_t perp = abs(normal * p - normal * mean) / sqrt(normal * normal);
  return perp * normal;
}

Double_t STHelixTrack::GetGenfitMomentum() const
{
  return fGenfitMomentum;
}

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
  if (fFitStatus == STHelixTrack::kHelix || fFitStatus == STHelixTrack::kGenfitTrack)
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

TVector3 STHelixTrack::GetMean()  const { return TVector3(fExpectationX, fExpectationY, fExpectationZ); }
Double_t STHelixTrack::GetXMean() const { return fExpectationX; }
Double_t STHelixTrack::GetYMean() const { return fExpectationY; }
Double_t STHelixTrack::GetZMean() const { return fExpectationZ; }
Double_t STHelixTrack::GetXCov()  const { return CovWXX()/fChargeSum; }
Double_t STHelixTrack::GetZCov()  const { return CovWZZ()/fChargeSum; }

Double_t STHelixTrack::CovWXX() const { return fChargeSum * (fExpectationXX - fExpectationX * fExpectationX); }
Double_t STHelixTrack::CovWYY() const { return fChargeSum * (fExpectationYY - fExpectationY * fExpectationY); }
Double_t STHelixTrack::CovWZZ() const { return fChargeSum * (fExpectationZZ - fExpectationZ * fExpectationZ); }

Double_t STHelixTrack::CovWXY() const { return fChargeSum * (fExpectationXY - fExpectationX * fExpectationY); }
Double_t STHelixTrack::CovWYZ() const { return fChargeSum * (fExpectationYZ - fExpectationY * fExpectationZ); }
Double_t STHelixTrack::CovWZX() const { return fChargeSum * (fExpectationZX - fExpectationZ * fExpectationX); }

Double_t STHelixTrack::GetExpectationX()  const { return fExpectationX; }
Double_t STHelixTrack::GetExpectationY()  const { return fExpectationY; }
Double_t STHelixTrack::GetExpectationZ()  const { return fExpectationZ; }

Double_t STHelixTrack::GetExpectationXX() const { return fExpectationXX; }
Double_t STHelixTrack::GetExpectationYY() const { return fExpectationYY; }
Double_t STHelixTrack::GetExpectationZZ() const { return fExpectationZZ; }

Double_t STHelixTrack::GetExpectationXY() const { return fExpectationXY; }
Double_t STHelixTrack::GetExpectationYZ() const { return fExpectationYZ; }
Double_t STHelixTrack::GetExpectationZX() const { return fExpectationZX; }

Double_t STHelixTrack::GetRMSW()       const { return fRMSW; }
Double_t STHelixTrack::GetRMSH()       const { return fRMSH; }
Double_t STHelixTrack::GetAlphaHead()  const { return fAlphaHead; }
Double_t STHelixTrack::GetAlphaTail()  const { return fAlphaTail; }

Bool_t STHelixTrack::IsPositiveChargeParticle()  const { return fIsPositiveChargeParticle; }



Int_t STHelixTrack::GetNumHits() const { return fMainHits.size(); }
STHit *STHelixTrack::GetHit(Int_t idx) const { return fMainHits.at(idx); }
std::vector<STHit *> *STHelixTrack::GetHitArray() { return &fMainHits; }

Int_t STHelixTrack::GetNumCandHits() const { return fCandHits.size(); }
std::vector<STHit *> *STHelixTrack::GetCandHitArray() { return &fCandHits; }

Int_t STHelixTrack::GetNumEmbedClusters() const
{
  Int_t num_embed = 0;
  for (auto cluster : fHitClusters)
    {
      if(cluster->IsEmbed() == true)
	num_embed++;
    }

  return num_embed;
}

Int_t STHelixTrack::GetNumClusters() const { return fHitClusters.size(); }
Int_t STHelixTrack::GetNumStableClusters() const
{
  auto count = 0;
  for (auto cluster : fHitClusters) {
    if (cluster -> IsStable())
      count++;
  }
  return count;
}
STHitCluster *STHelixTrack::GetCluster(Int_t idx) const { return fHitClusters.at(idx); }
std::vector<STHitCluster *> *STHelixTrack::GetClusterArray() { return &fHitClusters; }



Int_t STHelixTrack::GetNumHitIDs() const { return fMainHitIDs.size(); }
Int_t STHelixTrack::GetHitID(Int_t idx) const { return fMainHitIDs.at(idx); }
std::vector<Int_t> *STHelixTrack::GetHitIDArray() { return &fMainHitIDs; }

Int_t STHelixTrack::GetNumClusterIDs() const { return fClusterIDs.size(); }
Int_t STHelixTrack::GetClusterID(Int_t idx) const { return fClusterIDs.at(idx); }
std::vector<Int_t> *STHelixTrack::GetClusterIDArray() { return &fClusterIDs; }

std::vector<Double_t> *STHelixTrack::GetdEdxArray() { return &fdEdxArray; }



Double_t 
STHelixTrack::DistCircle(TVector3 pointGiven) const
{
  Double_t dx = pointGiven.X() - fXHelixCenter;
  Double_t dz = pointGiven.Z() - fZHelixCenter;
  return sqrt(dx*dx + dz*dz) - fHelixRadius;
}

Int_t STHelixTrack::Charge()   const { return fIsPositiveChargeParticle ? 1 : -1; }
Int_t STHelixTrack::Helicity() const { return fAlphaSlope > 0 ? 1 : -1; }

TVector3 STHelixTrack::PositionAtHead() const { return PositionByAlpha(fAlphaHead); }
TVector3 STHelixTrack::PositionAtTail() const { return PositionByAlpha(fAlphaTail); }

Double_t STHelixTrack::Momentum(Double_t B) const
{
  if (fFitStatus != STHelixTrack::kHelix && fFitStatus != STHelixTrack::kGenfitTrack)
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
  Double_t alphaPointer = alpha;

  Double_t alphaMid = (fAlphaHead + fAlphaTail) * 0.5;

  if (fAlphaHead > fAlphaTail)
    alphaPointer =+ TMath::Pi()/2.;
  else
    alphaPointer =- TMath::Pi()/2.;

  TVector3 posCenter(fXHelixCenter, 0, fZHelixCenter);
  TVector3 direction = PositionByAlpha(alpha) - posCenter;
  auto directionY = direction.Y();
  direction.SetY(0);
  direction.SetMag(2*TMath::Pi()*fHelixRadius);

  if (directionY > 0) direction.SetY(+abs(YLengthInPeriod()));
  else                direction.SetY(-abs(YLengthInPeriod()));

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

  Double_t yLengthInPeriod = std::abs(YLengthInPeriod());
  if (yLengthInPeriod > 3*fRMSH && yLengthInPeriod > 5 && std::abs(DipAngle()) < 1.5)
  {
    Int_t count = 0;
    while(1)
    {
      alpha1 = alpha1 + 2*TMath::Pi();
      point1.SetY(point1.Y() + 2*TMath::Pi()*fAlphaSlope);
      y1 = std::abs(point1.Y() - pointGiven.Y());

      if (std::abs(y0) < std::abs(y1))
        break;
      else {
        alpha0 = alpha1;
        point0 = point1;
        y0 = y1;
      }
      if (count++ > 20)
        break;
    }

    y1 = y0;
    alpha1 = alpha0;
    point1 = point0;

    count = 0;
    while(1)
    {
      alpha1 = alpha1 - 2*TMath::Pi();
      point1.SetY(point1.Y() - 2*TMath::Pi()*fAlphaSlope);
      y1 = std::abs(point1.Y() - pointGiven.Y());

      if (std::abs(y0) < std::abs(y1))
        break;
      else {
        alpha0 = alpha1;
        point0 = point1;
        y0 = y1;
      }
      if (count++ > 20)
        break;
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
STHelixTrack::ExtrapolateToX(Double_t x, Double_t alphaRef, TVector3 &pointOnHelix) const
{
  if (CheckExtrapolateToX(x) == false)
    return false;

  Double_t zOff = sqrt(fHelixRadius * fHelixRadius - (x - fXHelixCenter) * (x - fXHelixCenter));
  Double_t z1 = fZHelixCenter + zOff;
  Double_t z2 = fZHelixCenter - zOff;

  Double_t alpha = TMath::ATan2(z1-fZHelixCenter, x-fXHelixCenter);
  Double_t alphaTemp = alpha;
  Double_t d1Cand = std::abs(alphaTemp-alphaRef);
  Double_t d1Temp = d1Cand;

  while (1) {
    alphaTemp = alpha + 2*TMath::Pi();
    d1Temp = std::abs(alphaTemp-alphaRef);
    if (d1Temp >= d1Cand)
      break;
    else {
      alpha = alphaTemp;
      d1Cand = d1Temp;
    }
  }
  while (1) {
    alphaTemp = alpha - 2*TMath::Pi();
    d1Temp = std::abs(alphaTemp-alphaRef);
    if (d1Temp >= d1Cand)
      break;
    else {
      alpha = alphaTemp;
      d1Cand = d1Temp;
    }
  }
  pointOnHelix = PositionByAlpha(alpha);

  return true;
}

bool
STHelixTrack::ExtrapolateToZ(Double_t z, Double_t alphaRef, TVector3 &pointOnHelix) const
{
  if (CheckExtrapolateToZ(z) == false)
    return false;

  Double_t xOff = sqrt(fHelixRadius * fHelixRadius - (z - fZHelixCenter) * (z - fZHelixCenter));
  Double_t x1 = fXHelixCenter + xOff;
  Double_t x2 = fXHelixCenter - xOff;

  Double_t alpha = TMath::ATan2(z-fZHelixCenter, x1-fXHelixCenter);
  Double_t alphaTemp = alpha;
  Double_t d1Cand = std::abs(alphaTemp-alphaRef);
  Double_t d1Temp = d1Cand;

  while (1) {
    alphaTemp = alpha + 2*TMath::Pi();
    d1Temp = std::abs(alphaTemp-alphaRef);
    if (d1Temp >= d1Cand)
      break;
    else {
      alpha = alphaTemp;
      d1Cand = d1Temp;
    }
  }
  while (1) {
    alphaTemp = alpha - 2*TMath::Pi();
    d1Temp = std::abs(alphaTemp-alphaRef);
    if (d1Temp >= d1Cand)
      break;
    else {
      alpha = alphaTemp;
      d1Cand = d1Temp;
    }
  }
  pointOnHelix = PositionByAlpha(alpha);

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

bool
STHelixTrack::ExtrapolateToX(Double_t x, TVector3 &pointOnHelix) const
{
  TVector3 position1, position2;
  Double_t alpha1, alpha2;

  if (ExtrapolateToX(x, position1, alpha1, position2, alpha2) == false)
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
STHelixTrack::AlphaAtPosition(TVector3 p)
{
  Double_t alpha;
  TVector3 q(0,0,0);
  ExtrapolateToPointAlpha(p, q, alpha);
  return alpha;
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

Double_t 
STHelixTrack::GetdEdxWithCut(Double_t lowR, Double_t highR) const
{
  auto numPoints = fdEdxArray.size();

  Int_t idxLow = Int_t(numPoints * lowR);
  Int_t idxHigh = Int_t(numPoints * highR);

  numPoints = idxHigh - idxLow;
  if (numPoints < 3)
    return -1;

  Double_t dEdx = 0.;
  for (Int_t idEdx = idxLow; idEdx < idxHigh; idEdx++)
    dEdx += fdEdxArray[idEdx];
  dEdx = dEdx/numPoints;

  return dEdx;
}

void STHelixTrack::AddHitClusterAtFront(STHitCluster *cluster)
{
  fHitClusters.insert(fHitClusters.begin(), cluster);
}
