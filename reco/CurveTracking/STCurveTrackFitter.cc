/**
 * @author JungWoo Lee
 */

#include "STCurveTrackFitter.hh"
#include "STDebugLogger.hh"
#include <iostream>

using namespace std;

ClassImp(STCurveTrackFitter)

STCurveTrackFitter::STCurveTrackFitter()
{
}

STCurveTrackFitter::~STCurveTrackFitter()
{
}

void 
STCurveTrackFitter::FitAndSetCurveTrack(STCurveTrack* track)
{
  UpdateCurve(track);
  FitAndSetTrack(track);
}

void 
STCurveTrackFitter::UpdateCurve(STCurveTrack* track)
{
  while (track -> GetNumHits() - track -> GetNumHitsRemoved() > fNumHitsAtHead)
  {
    STHit *hit = track -> GetHit(track -> GetNumHits() - 1 - fNumHitsAtHead);
    track -> RemoveHit(hit);
  }
}

void STCurveTrackFitter::SetNumHitsAtHead(Int_t num) { fNumHitsAtHead = num; }
