#include "STCorrLinearTTGeo.hh"

#include <iostream>

using namespace std;

STCorrLinearTTGeo::STCorrLinearTTGeo(Int_t nHitsToFitTrack,
                                     Double_t dotDrectionCut,
                                     Double_t dotNormalCut)
{
  fMinNumHitsToFitTrack = nHitsToFitTrack;
  fDotDirectionCut = dotDrectionCut;
  fDotNormalCut = dotNormalCut;
}

Bool_t 
STCorrLinearTTGeo::Correlate(STLinearTrack *track1, 
                              STLinearTrack *track2,
                              Bool_t &survive, 
                              Double_t &matchQuality)
{
  survive = kFALSE;

  if (track1 -> GetNumHits() <= fMinNumHitsToFitTrack ||
      track2 -> GetNumHits() <= fMinNumHitsToFitTrack)
    return kFALSE;

  if (track1 -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track1);

  if (track2 -> IsFitted() == kFALSE)
    fLTFitter -> FitAndSetTrack(track2);

  Double_t dotDirectionCut = fDotDirectionCut;
  Double_t dotNormalCut    = fDotNormalCut;

  Double_t dotDirection = track1 -> GetDirection().Dot(track2 -> GetDirection());
  Double_t dotNormal    = track1 -> GetNormal().Dot(track2 -> GetNormal());

  if (dotDirection < 0) dotDirection *= -1;
  if (dotNormal < 0)    dotNormal *= -1;

#ifdef DEBUGLIT_MERGE_CORR
  cout << "TTGeo: "
       << track1 -> GetDirection().Dot(track2 -> GetDirection()) << " >? "
       << dotDirectionCut << ", " 
       << track1 -> GetNormal().Dot(track2 -> GetNormal()) << " >? "
       << dotNormalCut << endl;
#endif

  if (dotDirection >= dotDirectionCut && dotNormal >= dotNormalCut) 
  {
    matchQuality = TMath::Sqrt(matchQuality*matchQuality + 
                               dotDirection*dotDirection + 
                               dotNormal*dotNormal)
                   / (dotDirection + dotNormal + matchQuality);

    survive = kTRUE;
  }

  return kTRUE;
}
