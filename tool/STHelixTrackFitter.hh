#ifndef STHELIXFITTER
#define STHELIXFITTER

#include <vector>
#include "STHit.hh"
#include "STHelixTrack.hh"
#include "ODRFitter.hh"

class STHelixTrackFitter
{
  public:
    STHelixTrackFitter() : fODRFitter(new ODRFitter()) {}
    ~STHelixTrackFitter() {};

    bool FitPlane(STHelixTrack *track);
    bool Fit(STHelixTrack *track);
    bool FitCluster(STHelixTrack *track);

  private:
    ODRFitter *fODRFitter;

  ClassDef(STHelixTrackFitter, 2)
};

#endif
