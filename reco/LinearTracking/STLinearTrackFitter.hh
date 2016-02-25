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

    /// Fit track and set variables.
    /// @param track pointer to STLinearTrack
    /// @param hit   pointer to STHit - NULL is possible
    /// @param rmsL  RMS of fitted line
    /// @param rmsP  RMS of fitted plane
    void Fit(STLinearTrack* track, STHit* hit, 
             Double_t &rmsL, Double_t &rmsP);
    void Fit(STLinearTrack* track1, STLinearTrack* track2,
             Double_t &rmsL, Double_t &rmsP);

    /// Fit and set track variables with fitted result
    void FitAndSetTrack(STLinearTrack* track);
    void MergeAndSetTrack(STLinearTrack* track1, STLinearTrack* track2);

    /// Set data for fitting
    virtual void SetDataToODRFitter(STLinearTrack* track);
    virtual void SetDataToODRFitter(STLinearTrack* track, STHit* hit);
    virtual void SetDataToODRFitter(STLinearTrack* track1, STLinearTrack* track2);

    void SortHits(STLinearTrack* track);

    /// Calculate and return perpendicular vector to the track line
    /// (having the shortest distance from track to hit.)
    TVector3 PerpLine(STLinearTrack* track, STHit* hit);
    TVector3 PerpLine(STLinearTrack* track, TVector3 hitPos);
    Double_t PerpDistLine(STLinearTrack* track, TVector3 hitPos);

    /// Calculate and return perpendicular vector to the track plane
    TVector3 PerpPlane(STLinearTrack* track, STHit* hit);
    TVector3 PerpPlane(STLinearTrack* track, TVector3 hitPos);

    /// Get closest point on the track from the hit
    TVector3 GetClosestPointOnTrack(STLinearTrack* track, STHit* hit);
    TVector3 GetClosestPointOnTrack(STLinearTrack* track, TVector3 posHit);

    /// Get point on the track due to x, y, z
    TVector3 GetPointOnX(STLinearTrack* track, Double_t x);
    TVector3 GetPointOnY(STLinearTrack* track, Double_t y);
    TVector3 GetPointOnZ(STLinearTrack* track, Double_t z);

    Double_t GetLengthOnTrack(STLinearTrack* track, STHit* hit);

  protected:
    ODRFitter* fODRFitter;

  ClassDef(STLinearTrackFitter, 3)
};

class STHitSortByTrackDirection 
{
  private:
    TVector3 centroid;
    TVector3 direction;

  public:
    STHitSortByTrackDirection(TVector3 c, TVector3 d) : centroid(c), direction(d) {}

    Bool_t operator() (STHit* hit1, STHit *hit2)
    { return ( (hit1 -> GetPosition() - centroid).Dot(direction) <
               (hit2 -> GetPosition() - centroid).Dot(direction) ); }
};

#endif
