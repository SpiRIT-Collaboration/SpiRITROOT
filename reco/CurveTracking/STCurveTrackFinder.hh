/**
 * @brief  Curve track finder class
 *
 * @author JungWoo Lee
 */

#ifndef STCURVETRACKFINDER
#define STCURVETRACKFINDER

#include "STVTrackFinder.hh"
#include "STCurveTrackingConf.hh"
#include "STCurveTrackFitter.hh"
#include "TClonesArray.h"
#include <vector>

class STCurveTrackFinder : public STVTrackFinder
{
  public:
    STCurveTrackFinder();
    ~STCurveTrackFinder() {}

    void BuildTracks(STEvent *event, TClonesArray *trackArray);
    void BuildTracks(TClonesArray *hitArray, TClonesArray *trackArray);

    void Init(STEvent *event, TClonesArray *trackArray);
    void Init(TClonesArray *hitArray, TClonesArray *trackArray);
    Bool_t AnaNextHit();

  private:
    STCurveTrackFitter *fFitter;

    std::vector<STHit *> *fHitBuffer;
    std::vector<STHit *> *fHitBufferAna;
    std::vector<STCurveTrack *> *fTrackBuffer;
    std::vector<STCorrCurveTH *> *fTrackHitCorrelators;

    TClonesArray *fTrackArrayAna;
    TClonesArray *fTrackArrayTemp1;
    TClonesArray *fTrackArrayTemp2;

    Int_t fNumTracks;


  ClassDef(STCurveTrackFinder, 1)
};

#endif
