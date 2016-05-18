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

    void SetNumTracksLowLimit(Int_t limit);

  private:
    TClonesArray *fHitArray = nullptr;
    TClonesArray *fTrackArray = nullptr;

    STCurveTrackFinder* fTrackFinder;

    Int_t fNumTracksLowLimit = 1;

  ClassDef(STCurveTrackingETask, 1)
};

#endif
