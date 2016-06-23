#ifndef STPADPLANEMAP
#define STPADPLANEMAP

#include "STHit.hh"
#include <vector>

class STPadHitContainer;

typedef std::vector<STPadHitContainer *> pads_t;
typedef std::vector<STHit *> hits_t;

class STPadHitContainer
{
  public:
    STPadHitContainer() {};
    ~STPadHitContainer() {};

    void Clear();

    void AddNeighbor(STPadHitContainer *pad);
    pads_t *GetNeighbors();

    void AddHit(STHit *hit);
    STHit *PullOutNextHit();
    STHit *PullOutNextFreeHit();
    void PullOutHits(hits_t *array);
    void GetHits(hits_t *array);

  private:
    hits_t fHitArray;
    pads_t fNeighbors;
};

class STPadPlaneMap
{
  public:
    STPadPlaneMap();
    ~STPadPlaneMap() {};

    void Clear();

    void AddHit(STHit *hit);
    void AddHits(hits_t *hits);

    STHit *PullOutNextHit();
    STHit *PullOutNextFreeHit();
    void PullOutNeighborHits(hits_t *hits, hits_t *array, Int_t level = 0);
    void PullOutNeighborHits(STHit *hit, hits_t *array);
    void PullOutNeighborHits(TVector3 pos, Double_t range, hits_t *array);
    void PullOutPadHits(Int_t row, Int_t layer, hits_t *array);
    void PullOutRowHits(Int_t row, hits_t *array);
    void PullOutLayerHits(Int_t layer, hits_t *array);

    void GetNeighborHits(STHit *hit, hits_t *array);
    void GetRowHits(Int_t row, hits_t *array);
    void GetLayerHits(Int_t layer, hits_t *array);

    Int_t CalculateRow(Double_t x);
    Int_t CalculateLayer(Double_t z);
    Double_t CalculateX(Int_t row);
    Double_t CalculateZ(Int_t layer);

  private:
    STPadHitContainer *fPadMap[108][112];

    Int_t fNextRow = 107;
    Int_t fNextLayer = 111;

    Int_t fNextFreeRow = 107;
    Int_t fNextFreeLayer = 111;

  ClassDef(STPadPlaneMap, 1)
};

#endif
