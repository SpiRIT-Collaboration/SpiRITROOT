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
  fTrackID  = -1;
  fParentID = -99999;

  fFitStatus = kNon;

  fXHelixCenter = -99999;
  fZHelixCenter = -99999;
  fHelixRadius  = -99999;
  fDipAngle     = -99999;
  fYInitial     = -99999;

  fChargeSum = 0;

  fXMean = 0;
  fYMean = 0;
  fZMean = 0;
  fXCov  = 0;
  fZCov  = 0;

  if (TString(option) == "C")
    DeleteHits();
  else
    fHitArray.clear();
}

void STHelixTrack::Print(Option_t *option) const
{
  cout << left << endl;
  cout << "[STHelixTrack] - Units in [mm] [radian] [ADC]" << endl;

  Int_t lnm = 13, len = 5;
  TString dmy = "";

  cout << "" << setw(lnm) << "Track ID" << " : " << setw(len) << fTrackID << endl;
  cout << "" << setw(lnm) << "Parent ID" << " : " << setw(len) << fParentID << endl;
  if      (fFitStatus == STHelixTrack::kNon)   dmy = "Not Set";
  else if (fFitStatus == STHelixTrack::kHelix) dmy = "Helix";
  else if (fFitStatus == STHelixTrack::kLine)  dmy = "Straight Line";
  cout << "" << setw(lnm) << "Fit status" << " : " << setw(len) << dmy << endl;

  len = 12;
  dmy = "("+TString::Itoa(fXHelixCenter,10)+", x, "+TString::Itoa(fZHelixCenter,10)+")";
  cout << "" << setw(lnm) << "Helix Center" << " : " << setw(len) << dmy << endl;
  cout << "" << setw(lnm) << "Helix Radius" << " : " << setw(len) << fHelixRadius << endl;
  cout << "" << setw(lnm) << "Dip Angle" << " : " << setw(len) << fDipAngle << endl;
  cout << "" << setw(lnm) << "Initial Y" << " : " << setw(len) << fYInitial << endl;

  cout << "" << setw(lnm) << "No. of Hits" << " : " << setw(len) << this -> GetNumHits() << endl;
  cout << "" << setw(lnm) << "Charge Sum" << " : " << setw(len) << fChargeSum << endl;
  dmy = "("+TString::Itoa(fXMean,10)+", "+TString::Itoa(fYMean,10)+", "+TString::Itoa(fZMean,10)+")";
  cout << "" << setw(lnm) << "Position Mean" << " : " << setw(len) << dmy << endl;
  dmy = "("+TString::Itoa(fXCov,10)+", x, "+TString::Itoa(fZCov,10)+")";
  cout << "" << setw(lnm) << "Position Cov." << " : " << setw(len) << dmy << endl;
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

  fHitArray.push_back(hit);
}

void STHelixTrack::DeleteHits()
{
  for (auto hit : fHitArray)
    delete hit;

  fHitArray.clear();
}

void STHelixTrack::SetFitStatus(STFitStatus value) { fFitStatus = value; }
void STHelixTrack::SetStatusToHelix() { fFitStatus = STHelixTrack::kHelix; }
void STHelixTrack::SetStatusToLine()  { fFitStatus = STHelixTrack::kLine; }
void STHelixTrack::SetHelixCenter(Double_t x, Double_t z) { fXHelixCenter = x; fZHelixCenter = z; }
void STHelixTrack::SetHelixRadius(Double_t r) { fHelixRadius = r; }
void STHelixTrack::SetDipAngle(Double_t dip)  { fDipAngle = dip; }
void STHelixTrack::SetYInitial(Double_t y)    { fYInitial = y; }

STHelixTrack::STFitStatus STHelixTrack::GetFitStatus()  { return fFitStatus; }

Double_t STHelixTrack::GetHelixCenterX()  { return fXHelixCenter; }
Double_t STHelixTrack::GetHelixCenterZ()  { return fZHelixCenter; }
Double_t STHelixTrack::GetHelixRadius()   { return fHelixRadius; }
Double_t STHelixTrack::GetDipAngle()      { return fDipAngle; }
Double_t STHelixTrack::GetYInitial()      { return fYInitial; }


void STHelixTrack::GetHelixParameters(Double_t &xCenter, 
                                      Double_t &zCenter, 
                                      Double_t &radius, 
                                      Double_t &dipAngle,
                                      Double_t &yInitial)
{
  if (fFitStatus == STHelixTrack::kHelix)
  {
    xCenter  = fXHelixCenter;
    zCenter  = fZHelixCenter;
    radius   = fHelixRadius;
    dipAngle = fDipAngle;
    yInitial = fYInitial;
  }
  else
  {
    xCenter  = -99999;
    zCenter  = -99999;
    radius   = -99999;
    dipAngle = -99999;
    yInitial = -99999;
  }
}

/*
void STHelixTrack::GetLineParameters(Double_t &xDirection, 
                                     Double_t &yDirection, 
                                     Double_t &zDirection,
                                     Double_t &xVertex,
                                     Double_t &yVertex,
                                     Double_t &zVertex)
{
  xDirection = fXHelixCenter;
  yDirection = fYHelixCenter;
  zDirection = fZHelixCenter;
  xVertex;
  yVertex;
  zVertex;
}
*/

Double_t STHelixTrack::GetChargeSum()  { return fChargeSum; }
Double_t STHelixTrack::GetXMean()      { return fXMean; }
Double_t STHelixTrack::GetYMean()      { return fYMean; }
Double_t STHelixTrack::GetZMean()      { return fZMean; }
Double_t STHelixTrack::GetXCov()       { return fXCov; }
Double_t STHelixTrack::GetZCov()       { return fZCov; }

Int_t STHelixTrack::GetNumHits() const { return fHitArray.size(); }
std::vector<STHit *> *STHelixTrack::GetHitArray() { return &fHitArray; }
