#ifndef STHITCLUSTERINGCTTASK_HH
#define STHITCLUSTERINGCTTASK_HH

#include "STRecoTask.hh"
#include "STHit.hh"
#include "STHitCluster.hh"
#include "STClusterizer.hh"

class STHitClusteringCTTask : public STRecoTask
{
  public:
    STHitClusteringCTTask();
    STHitClusteringCTTask(Bool_t persistence);
    ~STHitClusteringCTTask();
    
    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetProximityCut(Double_t x, Double_t y, Double_t z);
    void SetSigmaCut(Double_t x, Double_t y, Double_t z);
    void SetEdgeCut(Double_t low, Double_t high);

    void SetClusterByLength(Bool_t flag);

  private:
    TClonesArray *fClusterArray = nullptr;
    TClonesArray *fTrackArray = nullptr;

    STClusterizer *fClusterizer;   //!< Clusterizer pointer

      Bool_t fSetProxCut = kFALSE;
    Double_t fXCut;
    Double_t fYCut;
    Double_t fZCut;

      Bool_t fSetSigmaCut = kFALSE;
    Double_t fSigmaXCut;
    Double_t fSigmaYCut;
    Double_t fSigmaZCut;

      Bool_t fSetEdgeCut = kFALSE;
    Double_t fXLowCut;
    Double_t fXHighCut;

    Bool_t fClusterByLength = kFALSE;

  ClassDef(STHitClusteringCTTask, 1)
};

#endif
