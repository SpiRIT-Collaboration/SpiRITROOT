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
    ~STPSATask();

    enum STPSAMode { kSimple, kAll, kLayer, kOPTICS ,kDF, kFast};

    void SetPSAMode(STPSAMode mode);
    void SetThreshold(Double_t threshold);
    void SetLayerCut(Double_t layerCut);

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    STPSA* GetPSA();

  private:
    TClonesArray *fRawEventArray;
    TClonesArray *fEventArray;

    STPSA *fPSA;         //!< Pulse shape analyzer
    STPSAMode fPSAMode;
    
    Double_t fThreshold;
    Int_t    fLayerCut;

    FairLogger *fLogger;   //!
    

  ClassDef(STPSATask, 1);
};

#endif
