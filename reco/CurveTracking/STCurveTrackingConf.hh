#ifndef STCURVETRACKINGCONF
#define STCURVETRACKINGCONF

#include "STHit.hh"
#include "STCurveTrack.hh"
#include "STCurveTrackFitter.hh"
#include "STDebugLogger.hh"
#include "STGlobal.hh"

class STCorrCurveTH 
{
  protected:
    STCurveTrackFitter* fLTFitter;

  public:
    STCorrCurveTH():fLTFitter(new STCurveTrackFitter()) {}
    STCorrCurveTH(STCurveTrackFitter *fitter):fLTFitter(fitter) {}
    virtual ~STCorrCurveTH() {}

    virtual Bool_t Correlate(STCurveTrack *track, 
                             STHit *hit, 
                             Bool_t &survive, 
                             Double_t &matchQuality) = 0;
};
#endif
