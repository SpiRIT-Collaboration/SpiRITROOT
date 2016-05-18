/**
 * @brief Analyzing pulse shape of raw signal and make it to a hit
 *
 * Author List:
 * @author Genie Jhang (Korea University), original author
 * @author JungWoo Lee (Korea University)
 */

#ifndef STPSATASK_HH
#define STPSATASK_HH

// FAIRROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STRawEvent.hh"
#include "STDigiPar.hh"
#include "STPSA.hh"

// ROOT classes
#include "TClonesArray.h" 

class STPSATask : public FairTask 
{
  public:
    STPSATask();
    STPSATask(Bool_t persistence, Double_t threshold);
    ~STPSATask();

    enum STPSAMode { kSimple, kAll, kLayer, kOPTICS ,kDF, kFast, kFastFit};

    void SetPSAMode(STPSAMode mode);
    void SetThreshold(Double_t threshold);
    void SetLayerCut(Int_t lowCut, Int_t highCut);

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    void SetPersistence(Bool_t value = kTRUE);

    STPSA* GetPSA();

    void SetPulserData(TString pulserData);
    void UseDefautPulserData(Int_t shapingTime);

  private:
    Bool_t fIsPersistence;  ///< Persistence check variable

    TClonesArray *fRawEventArray;
    TClonesArray *fEventArray;

    STPSA *fPSA;         //!< Pulse shape analyzer
    STPSAMode fPSAMode;
    
    Double_t fThreshold;
    Int_t    fLayerLowCut;
    Int_t    fLayerHighCut;

    STDigiPar *fPar;   //!
    FairLogger *fLogger;   //!
    
    TString fPulserDataName;
    Int_t fShapingTime;

  ClassDef(STPSATask, 1);
};

#endif
