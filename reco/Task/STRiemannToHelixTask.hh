#ifndef STRIEMANNTOHELIXTASK_HH
#define STRIEMANNTOHELIXTASK_HH

#include "STRecoTask.hh"
#include "STHit.hh"
#include "STHitCluster.hh"
#include "STRiemannTrack.hh"
#include "STHelixTrack.hh"
#include "STHelixTrackFinder.hh"

class STRiemannToHelixTask : public STRecoTask
{
  public:
    STRiemannToHelixTask(): STRecoTask("RtoH", 1, false) {}
    STRiemannToHelixTask(Bool_t b): STRecoTask("RtoH", 1, b) {}
    ~STRiemannToHelixTask() {}

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

  private:
    TClonesArray *fHitClusterArray = nullptr;
    TClonesArray *fRiemannArray = nullptr;
    TClonesArray *fHelixArray = nullptr;

    STHelixTrackFitter *fFitter = nullptr;

  ClassDef(STRiemannToHelixTask, 1)
};

#endif
