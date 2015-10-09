/**
 * @author JungWoo Lee
 */

#include "STLinearTrackFinder.hh"

#include "TVector3.h"
#include "TMath.h"

#include <iostream>

STLinearTrackFinder::STLinearTrackFinder()
{
  fPrimaryVertex = TVector3(0, -213.3, -35.2);
}

STLinearTrackFinder::~STLinearTrackFinder()
{
}

void STLinearTrackFinder::BuildTracks(STEvent* event,
                                      std::vector<STLinearTrack*> *trackBuffer)
{
  fTrackBuffer = trackBuffer;
  Int_t nHits = event -> GetNumHits();

  std::vector<STHit*> *fHitBuffer;
  for (Int_t iHit = 0; iHit < nHits; iHit++) {
    STHit *hit = new STHit(event -> GetHit(iHit));
    fHitBuffer -> push_back(hit);
  }
  
  std::sort(fHitBuffer -> begin(), fHitBuffer -> end(), STLTFHitSortTheta());

  for (Int_t iHit = 0; iHit < nHits; iHit++) {

    STHit *hit = fHitBuffer -> at(iHit);

    Int_t nTracks = fTrackBuffer -> size();
    for(Int_t iTrack = 0; iTrack < nTracks; iTrack++) {
      
      STLinearTrack* track = fTrackBuffer -> at(iTrack);

      Bool_t survive;

      survive = ProximityCorrelator(track, hit);
      if (survive == kFALSE)
        continue;

      survive = QualityTest(track, hit);
      if (survive == kFALSE)
        continue;

      //fLinearTrackFitter -> Merge(track, hit);
    }
  }
}

Bool_t 
STLinearTrackFinder::ProximityCorrelator(STLinearTrack* track, STHit* hit)
{
  return kTRUE;
}

Bool_t 
STLinearTrackFinder::QualityTest(STLinearTrack* track, STHit* hit)
{
  return kTRUE;
}
