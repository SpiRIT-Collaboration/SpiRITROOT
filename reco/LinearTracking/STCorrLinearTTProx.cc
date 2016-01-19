#include "STCorrLinearTTProx.hh"

Bool_t 
STCorrLinearTTProx::Correlate(STLinearTrack *track1, 
                              STLinearTrack *track2,
                              Bool_t &survive, 
                              Double_t &matchQuality)
{
  survive = kFALSE;

  if (track1 -> GetNumHits() < fMinNumHitsCut || 
      track2 -> GetNumHits() < fMinNumHitsCut)
    return kFALSE;

  Double_t scaling = 2 - matchQuality;

  Double_t proxCut = scaling * fProxCut;

  if (track1 -> IsSorted() == kFALSE)
    fLTFitter -> SortHits(track1);

  if (track2 -> IsSorted() == kFALSE)
    fLTFitter -> SortHits(track2);

  TVector3 direction1 = track1 -> GetDirection();

  TVector3 centroid1 = track1 -> GetCentroid();
  TVector3 centroid2 = track2 -> GetCentroid();

  TVector3 vertex1i  = track1 -> GetVertex(0);
  TVector3 vertex1f  = track1 -> GetVertex(1);
  TVector3 vertex2i  = track2 -> GetVertex(0);
  TVector3 vertex2f  = track2 -> GetVertex(1);

  // u-axis defined with direction vector of track1

  Double_t ulowCut  = 0 - proxCut;
  Double_t uhighCut = direction1.Dot(vertex1f - vertex1i) + proxCut;

  Double_t uV2i = direction1.Dot(vertex2i  - vertex1i);
  Double_t uV2f = direction1.Dot(vertex2f  - vertex1i);
  Double_t uC2  = direction1.Dot(centroid2 - vertex1i);

  if ( (uV2i > ulowCut && uV2i < uhighCut) ||
       (uV2f > ulowCut && uV2f < uhighCut) ||
       (uC2  > ulowCut && uC2  < uhighCut) )
    survive = kTRUE;

  return kTRUE;
}
