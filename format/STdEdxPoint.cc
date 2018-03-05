#include "STdEdxPoint.hh"
#include <iomanip>
#include <iostream>
using namespace std;

ClassImp(STdEdxPoint)

STdEdxPoint::STdEdxPoint()
:STdEdxPoint(-1, -1, -1, -999)
{
}

STdEdxPoint::STdEdxPoint(Int_t id, Double_t de, Double_t dx, Double_t length)
:fClusterID(id), fdE(de), fdx(dx), fLength(length)
{
}

void STdEdxPoint::Print(Option_t *option) const
{
  cout << fClusterID << ": " << fdE/fdx << " (" << fdE << "/" << fdx << ") at " << fLength << endl;
}
