#ifndef STDEDXPOINT_HH
#define STDEDXPOINT_HH

#include "TObject.h"
#include "TVector3.h"

class STdEdxPoint : public TObject {
  public:
    STdEdxPoint();
    STdEdxPoint(Int_t id, Double_t de, Double_t dx, Double_t length, Int_t nHits, Int_t sHits, Bool_t isRow, Bool_t isContinuousHits, Int_t clusterSize);
    virtual ~STdEdxPoint() {};

    virtual void Print(Option_t *option = "") const;

    Int_t fClusterID;   ///< Hit-Cluster ID
    Int_t fNumHits;     ///< Number of hits in cluster
    Int_t fNumShadowHits;     ///< Number of hits possibly shaddowed by neighboring saturated hits
    Double_t fdE;       ///< (ADC)
    Double_t fdx;       ///< (mm)
    Double_t fLength;   ///< Length from vertex to this point (mm)
    TVector3 fPosition; ///<
    TVector3 fGenfitPos; ///<
    Bool_t fisRow;      ///< true if clustered by row

    Bool_t fIsContinuousHits; ///< Hit distribution of the cluster is continuous?
    Int_t fClusterSize;       ///< # of hits used to the cluster

  ClassDef(STdEdxPoint, 4)
};

class STdEdxPointSortByLength {
  public:
    bool operator() (STdEdxPoint p1, STdEdxPoint p2) { return p1.fLength < p2.fLength; }
};

#endif
