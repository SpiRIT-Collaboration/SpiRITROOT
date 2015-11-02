#ifndef STLINEARTRACKINGCONF
#define STLINEARTRACKINGCONF

//#define DEBUGLIT
//#define DEBUGLIT_BUILD
//#define DEBUGLIT_SELECT
//#define DEBUGLIT_MERGE
//#define DEBUGLIT_MERGE_CORR
//#define DEBUGLIT_TIMER
//#define DEBUGLIT_CUT

#include "STHit.hh"
#include "STLinearTrack.hh"
#include "STLinearTrackFitter.hh"
#include "STDebugLogger.hh"

class STCorrLinearTH 
{
  protected:
    STLinearTrackFitter* fLTFitter;

  public:
    STCorrLinearTH():fLTFitter(new STLinearTrackFitter()) {}
    virtual ~STCorrLinearTH() {}

    virtual Bool_t Correlate(STLinearTrack *track, 
                             STHit *hit, 
                             Bool_t &survive, 
                             Double_t &matchQuality) = 0;
};

class STCorrLinearTT 
{
  protected:
    STLinearTrackFitter* fLTFitter;

  public:
    STCorrLinearTT():fLTFitter(new STLinearTrackFitter()) {}
    virtual ~STCorrLinearTT() {}

    virtual Bool_t Correlate(STLinearTrack *track1, 
                             STLinearTrack *track2, 
                             Bool_t &survive, 
                             Double_t &matchQuality) = 0;
};

class STTestLinearTrack 
{
  protected:
    STLinearTrackFitter* fLTFitter;

  public:
    STTestLinearTrack():fLTFitter(new STLinearTrackFitter()) {}
    virtual ~STTestLinearTrack() {}

    virtual Bool_t Correlate(STLinearTrack *track, 
                             Bool_t &survive, 
                             Double_t &matchQuality) = 0;
};

#endif
