#ifndef STNLDIGITASK
#define STNLDIGITASK

#include "FairTask.h"
#include "STMCPoint.hh"
#include "TClonesArray.h"

#include "STNeuLAND.hh"

class STNLDigiTask : public FairTask
{
  public:
    STNLDigiTask(TString name = "");
    ~STNLDigiTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t* opt);
    virtual void SetParContainers();

    void SetBarPersistence(Bool_t value);
    void SetHitPersistence(Bool_t value);
    void SetHitClusterPersistence(Bool_t value);

  private:
    TString fName;
    Int_t fEventID;

    TClonesArray* fMCPointArray;

    TClonesArray* fBarArray;
    TClonesArray* fNLHitArray;
    TClonesArray* fNLHitClusterArray;

    STNeuLAND *fNL;

    Bool_t fIsBarPersistence = true;
    Bool_t fIsHitPersistence = true;
    Bool_t fIsHitClusterPersistence = true;

  ClassDef(STNLDigiTask,1);
};

#endif
