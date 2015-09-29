#ifndef STHITCLUSTERRICH
#define STHITCLUSTERRICH

#include "STHit.hh"

#include "TObject.h"
#include "TVector3.h"
#include "TMatrixD.h"

#include <vector>

#include "STHitCluster.hh"

using std::vector;

class STHitClusterRich : public STHitCluster 
{
  public:
    STHitClusterRich();
    STHitClusterRich(STHitClusterRich *cluster);

    void AddHit(STHit *hit);

    Double_t GetXMax();
    Double_t GetYMax();
    Double_t GetZMax();

    Double_t GetXMin();
    Double_t GetYMin();
    Double_t GetZMin();

  private:
    Double_t fXMax;
    Double_t fYMax;
    Double_t fZMax;

    Double_t fXMin;
    Double_t fYMin;
    Double_t fZMin;

  ClassDef(STHitClusterRich, 1);
};

#endif
