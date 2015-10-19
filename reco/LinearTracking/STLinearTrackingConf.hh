#ifndef STLINEARTRACKINGCONF
#define STLINEARTRACKINGCONF

#include "STHit.hh"
#include "STLinearTrack.hh"
#include "STLinearTrackFitter.hh"

class STLinearTrackingConf
{
  public:
    enum STLinearTrackSort
    {
      kSortX,
      kSortY,
      kSortZ,
      kSortR,
      kSortCharge
    };
};

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

#endif
