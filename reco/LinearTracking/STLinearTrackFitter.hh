/**
 * @author JungWoo Lee
 */

#ifndef STLINEARTRACKFITTER
#define STLINEARTRACKFITTER

#include "TVector3.h"

#include "STEvent.hh"
#include "STHit.hh"
#include "STLinearTrack.hh"

#include "TVirtualFitter.hh"

#include <vector>

class STLinearTrackFitter
{
  public:
    STLinearTrackFitter();
    ~STLinearTrackFitter();

    /// Fit track and set variables. Return quality of the fitting.
    Double_t Fit(STLinearTrack* track);
    /// Calculate and return perpendicular vector (having the shortest distance from track to hit.)
    Tvector3 Perp(STLinearTrack* track, STHit* hit);

    /// Merge track and hit. Run Fit() automatically.
    void Merge(STLinearTrack* track, STHit* hit);

  private:

    TVirtualFitter* fFitter;

  ClassDef(STLinearTrackFitter, 1)
};

#endif
