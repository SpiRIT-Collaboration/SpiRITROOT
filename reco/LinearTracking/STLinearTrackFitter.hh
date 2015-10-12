/**
 * @author JungWoo Lee
 */

#ifndef STLINEARTRACKFITTER
#define STLINEARTRACKFITTER

#include "TVector3.h"

#include "STEvent.hh"
#include "STHit.hh"
#include "STLinearTrack.hh"

#include "ODRFitter.hh"

#include <vector>

class STLinearTrackFitter
{
  public:
    STLinearTrackFitter();
    ~STLinearTrackFitter();

    /// Fit track and set variables. Return quality of the fitting
    Double_t Fit(STLinearTrack* track);

    /// Fit track and set variables. Return RMS of the fitting
    Double_t Fit(STLinearTrack* track, STHit* hit);

    /// Calculate and return perpendicular vector (having the shortest distance from track to hit.)
    TVector3 Perp(STLinearTrack* track, STHit* hit);
    TVector3 Perp(STLinearTrack* track, TVector3 hitPos);

    /// Get Closest point on the track from the hit
    TVector3 GetClosestPointOnTrack(STLinearTrack* track, STHit* hit);
    TVector3 GetClosestPointOnTrack(STLinearTrack* track, TVector3 posHit);

    /// Fit and set track variables with fitted result
    void FitAndSetTrack(STLinearTrack* track);

  private:
    ODRFitter* fODRFitter;

  ClassDef(STLinearTrackFitter, 1)
};

#endif
