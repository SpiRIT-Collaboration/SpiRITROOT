#include "STdEdxPoint.hh"

ClassImp(STdEdxPoint)

STdEdxPoint::STdEdxPoint()
:STdEdxPoint(-1, -1, -1)
{
}

STdEdxPoint::STdEdxPoint(Int_t groupID, Double_t de, Double_t dx)
:fGroupID(groupID), fdE(de), fdx(dx)
{
}
