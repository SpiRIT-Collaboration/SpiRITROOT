#include "STdEdxPoint.hh"
#include <iomanip>
#include <iostream>
using namespace std;

ClassImp(STdEdxPoint)

STdEdxPoint::STdEdxPoint()
:STdEdxPoint(-1, -1, -1, -999, -1, -1, false, 0, -1)
{
}

STdEdxPoint::STdEdxPoint(Int_t id, Double_t de, Double_t dx, Double_t length, Int_t num_hits, Int_t shadow_hits, Bool_t isRow, Bool_t isContinuousHits, Int_t clusterSize)
:fClusterID(id), fdE(de), fdx(dx), fLength(length), fNumHits(num_hits), fNumShadowHits(shadow_hits), fisRow(isRow), fIsContinuousHits(isContinuousHits), fClusterSize(clusterSize)
{
}

void STdEdxPoint::Print(Option_t *option) const
{
  cout << fClusterID << ": " << fdE/fdx << " (" << fdE << "/" << fdx << ") at " << fLength << endl;
  cout << "fIsContinuousHits:" << (fIsContinuousHits ? "true" : "false") << endl;
  cout << "fClusterSize:" << fClusterSize << endl;
}
