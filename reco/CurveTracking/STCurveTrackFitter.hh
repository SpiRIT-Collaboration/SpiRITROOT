/**
 * @author JungWoo Lee
 */

#ifndef STCURVETRACKFITTER
#define STCURVETRACKFITTER

#include "STLinearTrackFitter.hh"
#include "STCurveTrack.hh"

class STCurveTrackFitter : public STLinearTrackFitter
{
  public:
    STCurveTrackFitter();
    ~STCurveTrackFitter();

    void FitAndSetCurveTrack(STCurveTrack* track);

    void SetNumHitsAtHead(Int_t num);

  private:
    void UpdateCurve(STCurveTrack* track);

    Int_t fNumHitsAtHead;


  ClassDef(STCurveTrackFitter, 1)
};

#endif
