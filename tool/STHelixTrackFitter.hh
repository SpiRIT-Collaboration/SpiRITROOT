#ifndef STHELIXFITTER
#define STHELIXFITTER

#include <vector>
#include "STHit.hh"
#include "STHelixTrack.hh"
#include "ODRFitter.hh"

class STHelixTrackFitter
{
  public:
    STHelixTrackFitter();
    ~STHelixTrackFitter() {};

    Bool_t Fit(STHelixTrack *track);

  private:
    Bool_t FitCircle(STHelixTrack *track);
    Bool_t FitLine(STHelixTrack *track);

  private:
    ODRFitter *fODRFitter;

  ClassDef(STHelixTrackFitter, 1)
};

#endif
