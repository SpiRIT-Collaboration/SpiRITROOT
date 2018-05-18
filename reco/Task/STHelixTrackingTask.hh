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
    void SetClusterCutLRTB(Double_t left, Double_t right, Double_t top, Double_t bottom);
  
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

    Double_t fCCLeft = 1000;
    Double_t fCCRight = -1000;
    Double_t fCCTop = 1000;
    Double_t fCCBottom = -1000;

  ClassDef(STHelixTrackingTask, 2)
};

#endif
