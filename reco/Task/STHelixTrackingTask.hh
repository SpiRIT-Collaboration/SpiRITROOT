#ifndef STHELIXTRACKINGETASK_HH
#define STHELIXTRACKINGETASK_HH

#include "STRecoTask.hh"
#include "STHit.hh"
#include "STHelixTrack.hh"
#include "STHelixTrackFinder.hh"

class STHelixTrackingTask : public STRecoTask
{
  public:
    STHelixTrackingTask();
    STHelixTrackingTask(Bool_t persistence);
    ~STHelixTrackingTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetNumTracksLowLimit(Int_t limit);

  private:
    TClonesArray *fHitArray = nullptr;
    TClonesArray *fTrackArray = nullptr;

    STHelixTrackFinder* fTrackFinder;

    Int_t fNumTracksLowLimit = 1;

  ClassDef(STHelixTrackingTask, 1)
};

#endif
