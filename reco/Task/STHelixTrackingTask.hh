#ifndef STHELIXTRACKINGETASK_HH
#define STHELIXTRACKINGETASK_HH

#include "STRecoTask.hh"
#include "STHit.hh"
#include "STHitCluster.hh"
#include "STHelixTrack.hh"
#include "STHelixTrackFinder.hh"

class STHelixTrackingTask : public STRecoTask
{
  public:
    STHelixTrackingTask();
    STHelixTrackingTask(Bool_t persistence, Bool_t clusterPersistence);
    ~STHelixTrackingTask();

    virtual void SetClusterPersistence(Bool_t value = kTRUE);

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetNumTracksLowLimit(Int_t limit);
    void SetClusteringOption(Int_t opt);
    void SetSaturationOption(Int_t opt);
  
    STHelixTrackFinder *GetTrackFinder();

  private:
    TClonesArray *fHitArray = nullptr;
    TClonesArray *fHitClusterArray = nullptr;
    TClonesArray *fTrackArray = nullptr;

    Bool_t fIsClusterPersistence = kFALSE;

    STHelixTrackFinder* fTrackFinder;

    Int_t fNumTracksLowLimit = 1;
    Int_t fClusteringOption = 2;
    Int_t fSaturationOption = 1;

  ClassDef(STHelixTrackingTask, 1)
};

#endif
