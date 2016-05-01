#include "STPadPlaneMap.hh"

void STPadMap::Clear()
{
  fHitArray.clear();
}

void STPadMap::AddNeighbor(STPadMap *neighbor)
{
  fNeighbors.push_back(neighbor);
}

pads_t *STPadMap::GetNeighbors() { return &fNeighbors; }

void STPadMap::AddHit(STHit *hit)
{
  fHitArray.push_back(hit);
}

void STPadMap::GetHits(hits_t *array)
{
  for (auto *hit : fHitArray)
    array -> push_back(hit);
}

void STPadMap::PullOutHits(hits_t *array)
{
  auto numHits = fHitArray.size();
  for (Int_t iHit = 0; iHit < numHits; iHit++) 
  {
    STHit *hit = fHitArray.back();
    fHitArray.pop_back();
    array -> push_back(hit);
  }
}

ClassImp(STPadPlaneMap)

STPadPlaneMap::STPadPlaneMap()
{
  for (Int_t row = 0; row < 108; row++)
    for (Int_t layer = 0; layer < 112; layer++) 
      fPadPlaneMap[row][layer] = new STPadMap();

  for (Int_t row = 0; row < 108; row++) {
    for (Int_t layer = 0; layer < 112; layer++) 
    {
      Int_t pm[] = {-1, 1};

      for (Int_t drow : pm) {
        for (Int_t dlayer : pm) 
        {
          Int_t rowNb = row + drow;
          Int_t layerNb = row + dlayer;

          if (rowNb < 0 || rowNb >= 108 || layerNb < 0 || layerNb >= 112 )
            continue;

          fPadPlaneMap[row][layer] -> AddNeighbor(fPadPlaneMap[rowNb][layerNb]);
        }
      }
    }
  }
}

void STPadPlaneMap::Clear()
{
  for (Int_t row = 0; row < 108; row++)
    for (Int_t layer = 0; layer < 112; layer++) 
      fPadPlaneMap[row][layer] -> Clear();

}

void STPadPlaneMap::AddHit(STHit *hit)
{
  fPadPlaneMap[hit->GetRow()][hit->GetLayer()] -> AddHit(hit);
}

void STPadPlaneMap::PullOutNeighborHits(STHit *hit, hits_t *array)
{
  auto *pad = fPadPlaneMap[hit->GetRow()][hit->GetLayer()];
  auto *neighbors = pad -> GetNeighbors();
  for (auto neighbor : *neighbors)
  {
    neighbor -> PullOutHits(array);
  }
}

void STPadPlaneMap::PullOutRowHits(Int_t row, hits_t *array)
{
  if (row < 0 || row >= 108)
    return;

  for (Int_t layer = 0; layer < 112; layer++) 
    fPadPlaneMap[row][layer] -> PullOutHits(array);
}

void STPadPlaneMap::PullOutLayerHits(Int_t layer, hits_t *array)
{
  if (layer< 0 || layer>= 112 )
    return;

  for (Int_t row = 0; row < 108; row++) 
    fPadPlaneMap[row][layer] -> PullOutHits(array);
}

void STPadPlaneMap::GetRowHits(Int_t row, hits_t *array)
{
  if (row < 0 || row >= 108)
    return;

  for (Int_t layer = 0; layer < 112; layer++) 
    fPadPlaneMap[row][layer] -> GetHits(array);
}

void STPadPlaneMap::GetLayerHits(Int_t layer, hits_t *array)
{
  if (layer< 0 || layer>= 112 )
    return;

  for (Int_t row = 0; row < 108; row++) 
    fPadPlaneMap[row][layer] -> GetHits(array);
}
