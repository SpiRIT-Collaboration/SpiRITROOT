#ifndef STCURVETRACKINGETASK_HH
#define STCURVETRACKINGETASK_HH

#include "STRecoTask.hh"
#include "STHit.hh"
#include "STCurveTrack.hh"
#include "STCurveTrackFinder.hh"

class STCurveTrackingETask : public STRecoTask
{
  public:
    STCurveTrackingETask();
    STCurveTrackingETask(Bool_t persistence);
    ~STCurveTrackingETask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

  private:
    TClonesArray *fHitArray = nullptr;
    TClonesArray *fTrackArray = nullptr;

    STCurveTrackFinder* fTrackFinder;

  ClassDef(STCurveTrackingETask, 1)
};

#endif
