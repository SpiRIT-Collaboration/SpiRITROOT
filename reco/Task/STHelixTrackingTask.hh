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
    void SetClusterCutLRTB(Double_t left, Double_t right, Double_t top, Double_t bottom);
    void SetCylinderCut(TVector3 center, Double_t radius, Double_t zLength, Double_t margin);
    void SetSphereCut(TVector3 center, Double_t radius, Double_t margin);
    void SetEllipsoidCut(TVector3 center, TVector3 radii, Double_t margin);

    void SetClusteringAngleAndMargin(Double_t angle, Double_t margin);
  
    STHelixTrackFinder *GetTrackFinder();

  private:
    TClonesArray *fHitArray = nullptr;
    TClonesArray *fHitClusterArray = nullptr;
    TClonesArray *fTrackArray = nullptr;

    Bool_t fIsClusterPersistence = kFALSE;

    STHelixTrackFinder* fTrackFinder;

    Int_t fNumTracksLowLimit = 1;
    Int_t fClusteringOption = 2;

    Double_t fCCLeft = 1000;
    Double_t fCCRight = -1000;
    Double_t fCCTop = 1000;
    Double_t fCCBottom = -1000;

    TVector3 fCutCenter = TVector3(-9999, -9999, -9999);
    Double_t fCRadius = -1;
    Double_t fZLength = -1;
    Double_t fSRadius = -1;
    TVector3 fERadii = TVector3(-1, -1, -1);
    Double_t fCutMargin = -1;

    Double_t fClusteringAngle = 45.;
    Double_t fClusteringMargin = 0.;

  ClassDef(STHelixTrackingTask, 2)
};

#endif
