#ifndef STPSAETASK_HH
#define STPSAETASK_HH

#include "STRecoTask.hh"
#include "STRawEvent.hh"
#include "STPSAFastFit.hh"

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
    virtual void SetParContainers();
  /// Running the task. This will be called when Run() method invoked from FairRun.

    void SetThreshold(Double_t threshold);
    void SetLayerCut(Int_t lowCut, Int_t highCut);

    void SetNumHitsLowLimit(Int_t limit);

    void SetPulserData(TString pulserData);
    void UseDefautPulserData(Int_t shapingTime);
    void SetEmbedding(Bool_t value);
    void SetEmbedFile(TString);
    void SetGainMatchingScale(Double_t val);
    void SetGainMatchingData(TString filename);
    void SetPSAPeakFindingOption(Int_t opt);

    void SetYOffsets(TString fileName);

    void SetYPedestalOffset(Double_t offset);

  private:
    TClonesArray *fRawEventArray = nullptr;
    TClonesArray *fRawEmbedEventArray = nullptr;
    TClonesArray *fRawDataEventArray = nullptr;
    TClonesArray *fHitArray = nullptr;
    TClonesArray *fEmbedHitArray = nullptr;    ///< hit array for embedded hits
    TClonesArray *fDataHitArray = nullptr;    ///< hit array for embedded hits
  
    TString fEmbedFile = "";                   ///< MC file for embedding
    STPSAFastFit *fPSA;
    Int_t fPSAPeakFindingOption = 1; //High efficiency set to defualt = 1; 0 is Jung Woo's org method
    TString fYOffsetFileName = "";
    Double_t fYPedestalOffset = 0;
    
    Double_t fThreshold = 30;
    Int_t fLayerLowCut  = -1;
    Int_t fLayerHighCut = 112;

    TString fPulserDataName = "";
    Int_t fShapingTime  = 117;

    Int_t fNumHitsLowLimit = 1;

    Double_t fGainMatchingScale = 1;
    TString fGainMatchingData = "";

    Bool_t fIsEmbedding = false;
    STDigiPar *fPar = NULL;

  ClassDef(STPSAETask, 1)
};

#endif
