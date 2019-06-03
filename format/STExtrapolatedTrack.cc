#include "STExtrapolatedTrack.hh"

#include <iostream>
using namespace std;

ClassImp(STExtrapolatedTrack)

STExtrapolatedTrack::STExtrapolatedTrack() 
{
  Clear();
}

void STExtrapolatedTrack::Clear(Option_t *option)
{
  TObject::Clear();

  fParentID = -99999;
  fTrackID = -99999;

  fPoints.clear();
}

void STExtrapolatedTrack::Print(Option_t *option) const
{
  cout << "STExtrapolatedTrack " << fTrackID << "(" << fParentID << ") with "
    << fPoints.size() << " points" << endl;;
}

void STExtrapolatedTrack::SetParentID(Int_t id) { fParentID = id; }
void STExtrapolatedTrack::SetTrackID(Int_t id) { fTrackID = id; } 
void STExtrapolatedTrack::AddPoint(TVector3 point, Double_t length)
{
  cout << fTrackID << " " << point.X() << " " << point.Y() << " " << point.Z() << " " << length << endl;

  if (length >= 0)
    fLengths.push_back(length);
  else if (fPoints.size() > 0) {
    auto last_point = fPoints.back();
    auto last_length = fLengths.back();
    length = last_length + (point-last_point).Mag();
    fLengths.push_back(length);
  }
  else {
    fLengths.push_back(0);
  }
  fPoints.push_back(point);

  cout << "==" << endl;
}

Int_t STExtrapolatedTrack::GetParentID() const { return fParentID; }
Int_t STExtrapolatedTrack::GetTrackID() const { return fTrackID; }
vector<TVector3> *STExtrapolatedTrack::GetPoints() { return &fPoints; }
vector<Double_t> *STExtrapolatedTrack::GetLengths() { return &fLengths; }

Int_t STExtrapolatedTrack::GetNumPoint() const { return (Int_t) fPoints.size(); }

TVector3 STExtrapolatedTrack::ExtrapolateByLength(Double_t length) const
{
  Int_t numPoints = fPoints.size();
  if (numPoints < 2)
    return TVector3();

  Double_t length1 = fLengths.at(0);
  Double_t length2 = fLengths.at(1);
  TVector3  point1 =  fPoints.at(0);
  TVector3  point2 =  fPoints.at(1);

  if (length < length1) {
    auto d1 = length - length1;
    auto d2 = length - length2;
    auto point = 1./(d1-d2) * (d1*point2 - d2*point1);
    return point;
  }

  for (auto iPoint=2; iPoint<numPoints; ++iPoint)
  {
    length1 = length2;
     point1 =  point2;

    length2 = fLengths.at(iPoint);
     point2 =  fPoints.at(iPoint);

    if (length2 > length){
      break;
    }
  }

  if (length < length2) {
    auto d1 = length - length1;
    auto d2 = length2 - length;
    auto point = 1./(d1+d2) * (d1*point2 + d2*point1);
    return point;
  }

  auto d1 = length - length1;
  auto d2 = length - length2;
  auto point = 1./(d1-d2) * (d1*point2 - d2*point1);
  return point;
}
