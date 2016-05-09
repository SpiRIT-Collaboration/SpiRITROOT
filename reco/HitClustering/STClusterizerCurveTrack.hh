#ifndef STCLUSTERIZERCURVETRACK
#define STCLUSTERIZERCURVETRACK

#include "STClusterizer.hh"
#include "STCurveTrackFitter.hh"
#include "STCircleFitter.hh"
#include "STRiemannFitter.hh"

#include "STHitCluster.hh"

#include "TMath.h"
#include "TVector3.h"

class STClusterizerCurveTrack : public STClusterizer
{
  public:
    STClusterizerCurveTrack();
    ~STClusterizerCurveTrack();

    void AnalyzeTrack(TClonesArray* trackArray, STEvent* eventOut);
    void AnalyzeTrack(TClonesArray *trackArray, TClonesArray *clusterArray);
    void AnalyzeSingleTrack(STCurveTrack *track, TClonesArray *clusterArray);

  private:
    STHitCluster* NewCluster(STHit* hit, TClonesArray *array);

    TClonesArray *fClusterArray;

    STCurveTrackFitter *fTrackFitter;
    STRiemannFitter *fRiemannFitter;

    STCurveTrack *fTracker;

  ClassDef(STClusterizerCurveTrack, 1)
};

#endif
