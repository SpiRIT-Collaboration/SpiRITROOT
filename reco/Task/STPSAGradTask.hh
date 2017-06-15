#ifndef STPSAGRADTASK_HH
#define STPSAGRADTASK_HH

#include "STRecoTask.hh"
#include "STRawEvent.hh"
#include "STPSAGrad.hh"

class STPSAGradTask : public STRecoTask 
{
  public:
    STPSAGradTask();
    ~STPSAGradTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetThreshold(Double_t threshold);
    void SetLayerCut(Int_t lowCut, Int_t highCut);

    void SetNumHitsLowLimit(Int_t limit);

  private:
    TClonesArray *fRawEventArray = nullptr;
    TClonesArray *fHitArray = nullptr;

    STPSAGrad *fPSA;
    
    Double_t fThreshold = 20;
    Int_t fLayerLowCut  = -1;
    Int_t fLayerHighCut = 112;

    Int_t fNumHitsLowLimit = 1;

  ClassDef(STPSAGradTask, 1)
};

#endif
