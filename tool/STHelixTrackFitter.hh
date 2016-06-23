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

    Bool_t Fit(STHelixTrack *track);

  private:
    ODRFitter *fODRFitter;

  ClassDef(STHelixTrackFitter, 1)
};

#endif
