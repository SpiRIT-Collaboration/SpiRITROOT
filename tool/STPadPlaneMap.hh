#ifndef STPADPLANEMAP
#define STPADPLANEMAP

#include "STHit.hh"
#include <vector>

class STPadMap;

typedef std::vector<STPadMap *> pads_t;
typedef std::vector<STHit *> hits_t;

class STPadMap
{
  public:
    STPadMap() {};
    ~STPadMap() {};

    void Clear();

    void AddNeighbor(STPadMap *pad);
    pads_t *GetNeighbors();

    void AddHit(STHit *hit);
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
    void PullOutNeighborHits(STHit *hit, hits_t *array);

    void PullOutRowHits(Int_t row, hits_t *array);
    void PullOutLayerHits(Int_t layer, hits_t *array);

    void GetRowHits(Int_t row, hits_t *array);
    void GetLayerHits(Int_t layer, hits_t *array);

  private:
    STPadMap *fPadPlaneMap[108][112];

  ClassDef(STPadPlaneMap, 1)
};

#endif
