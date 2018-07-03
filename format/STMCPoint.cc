#include "STMCPoint.hh"

#include <iostream>
using std::cout;
using std::endl;


// -----   Default constructor   -------------------------------------------
STMCPoint::STMCPoint()
  : FairMCPoint(), fPdg(0)
{
}
// -------------------------------------------------------------------------

// -----   Standard constructor   ------------------------------------------
STMCPoint::STMCPoint(Int_t trackID, Int_t detID,
      TVector3 pos, TVector3 mom,
      Double_t tof, Double_t length,
      Double_t eLoss, Int_t pdg)
  : FairMCPoint(trackID, detID, pos, mom, tof, length, eLoss), fPdg(pdg)
{
}
// -------------------------------------------------------------------------


// -----   Copy constructor   ------------------------------------------
STMCPoint::STMCPoint(const STMCPoint& point)
  : FairMCPoint(point.fTrackID, point.fDetectorID,
    TVector3(point.fX, point.fY, point.fZ), TVector3(point.fPx, point.fPy, point.fPz),
    point.fTime, point.fLength, point.fELoss, point.fEventId),
    fPdg(point.fPdg)
{}

// -----   Assignment operator    ------------------------------------------
STMCPoint& STMCPoint::operator= (const STMCPoint& point)
{
   if(this != &point){
    fTrackID = point.fTrackID;
    fDetectorID = point.fDetectorID;
    fX = point.fX;
    fY = point.fY;
    fZ = point.fZ;
    fPx = point.fPx;
    fPy = point.fPy;
    fPz = point.fPz;
    fTime = point.fTime;
    fLength = point.fLength;
    fELoss = point.fELoss;
    fEventId = point.fEventId;
    fPdg = point.fPdg;
   }

  return *this;
}


// -----   Destructor   ----------------------------------------------------
STMCPoint::~STMCPoint() { }
// -------------------------------------------------------------------------

// -----   Public method Print   -------------------------------------------
void STMCPoint::Print(const Option_t* opt) const
{
  cout << "-I- STMCPoint: SPiRIT point for track " << fTrackID
       << " in detector " << fDetectorID << endl;
  cout << "    Position (" << fX << ", " << fY << ", " << fZ
       << ") cm" << endl;
  cout << "    Momentum (" << fPx << ", " << fPy << ", " << fPz
       << ") GeV" << endl;
  cout << "    Time " << fTime << " ns,  Length " << fLength
       << " cm,  Energy loss " << fELoss*1.0e06 << " keV" << endl;
}
// -------------------------------------------------------------------------

ClassImp(STMCPoint)

