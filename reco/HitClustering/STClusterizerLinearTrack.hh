#ifndef STCLUSTERIZERLINEARTRACK
#define STCLUSTERIZERLINEARTRACK

#include "STClusterizer.hh"
#include "STLinearTrackFitter.hh"

#include "STHitCluster.hh"

#include "TMath.h"
#include "TVector3.h"

class STClusterizerLinearTrack : public STClusterizer
{
  public:
    STClusterizerLinearTrack();
    ~STClusterizerLinearTrack();

    void AnalyzeTrack(TClonesArray* trackArray, STEvent* eventOut);

  private:
    STHitCluster* NewCluster(STHit* hit);

    TClonesArray *fClusterArray;

    STLinearTrackFitter *fLinearTrackFitter;

  ClassDef(STClusterizerLinearTrack, 1)
};

#endif
