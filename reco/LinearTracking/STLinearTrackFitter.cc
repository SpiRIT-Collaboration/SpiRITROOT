/**
 * @author JungWoo Lee
 */

#include "STLinearTrackFitter.hh"

STLinearTrackFitter::STLinearTrackFitter()
{
}

STLinearTrackFitter::~STLinearTrackFitter()
{
}

Double_t
STLinearTrackFitter::Fit(STLinearTrack* track)
{
  std::vector<STHit*> *hitPointerBuffer = track -> GetHitPointerArray();
  Int_t nHits = hitPointerBuffer -> size();

  for (Int_t iHit = 0; iHit < nHits; iHit++) {
    STHit* hit = hitPointerBuffer -> at(iHit);
  }

  TVirtualFitter::SetDefaultFitter("Minuit");
  fFitter = TVirtualFitter::Fitter(0, 6);
  fFitter -> SetFCN

}

Double_t
STLinearTrackFitter::Perp(STLinearTrack* track, STHit* hit) 
{
  return -1;
}

void 
STLinearTrackFitter::Merge(STLinearTrack* track, STHit* hit) 
{
}
