#include "STdEdxPoint.hh"
#include <iomanip>
#include <iostream>
using namespace std;

ClassImp(STdEdxPoint)

STdEdxPoint::STdEdxPoint()
:STdEdxPoint(-1, -1)
{
}

STdEdxPoint::STdEdxPoint(Double_t de, Double_t dx)
:fdE(de), fdx(dx)
{
}

void STdEdxPoint::Print(Option_t *option) const
{
  cout << fdE/fdx << " (" << fdE << "/" << fdx << ")" << endl;
}
