#ifndef STPSAETASK_HH
#define STPSAETASK_HH

#include "STRecoTask.hh"
#include "STRawEvent.hh"
#include "STPSA.hh"

class STPSAETask : public STRecoTask 
{
  public:
    STPSAETask();

    STPSAETask(Bool_t persistence, 
               Int_t shapingTime,
               Double_t threshold = 0, 
               Int_t layerLowCut = -1,
               Int_t layerHighCut = -1);

    STPSAETask(Bool_t persistence,
               TString pulserData,
               Double_t threshold = 0,
               Int_t layerLowCut = -1,
               Int_t layerHighCut = -1);

    ~STPSAETask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetThreshold(Double_t threshold);
    void SetLayerCut(Int_t lowCut, Int_t highCut);

    void SetNumHitsLowLimit(Int_t limit);

    void SetPulserData(TString pulserData);
    void UseDefautPulserData(Int_t shapingTime);

  private:
    TClonesArray *fRawEventArray = nullptr;
    TClonesArray *fHitArray = nullptr;

    STPSA *fPSA;
    
    Double_t fThreshold = 20;
    Int_t fLayerLowCut  = -1;
    Int_t fLayerHighCut = 112;

    TString fPulserDataName = "";
    Int_t fShapingTime  = 117;

    Int_t fNumHitsLowLimit = 1;

  ClassDef(STPSAETask, 1)
};

#endif
