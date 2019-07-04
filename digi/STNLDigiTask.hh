#ifndef STNLDIGITASK
#define STNLDIGITASK

#include "FairTask.h"
#include "STMCPoint.hh"
#include "TClonesArray.h"

class STNLDigiTask : public FairTask
{
  public:
    STNLDigiTask();
    ~STNLDigiTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t* opt);
    virtual void SetParContainers();

    void SetHitPersistence(Bool_t value);
    void SetHitClusterPersistence(Bool_t value);

  private:
    TClonesArray* fMCPointArray;
    TClonesArray* fNLHitArray;
    TClonesArray* fNLHitClusterArray;

    Int_t fEventID;

    Bool_t fIsHitPersistence = true;
    Bool_t fIsHitClusterPersistence = true;


    Double_t fZTarget = -13.24; // mm
    Double_t fDistNeuland = 9093.85;
    Double_t fRotYNeuland_deg = 29.579;
    Double_t fRotYNeuland_rad = fRotYNeuland_deg*TMath::DegToRad();
    Double_t fOffxNeuland = fDistNeuland * sin( fRotYNeuland_rad ); 
    Double_t fOffyNeuland = 0.;   
    Double_t fOffzNeuland = fDistNeuland * cos( fRotYNeuland_rad ) + fZTarget;
    Double_t fdhwNl = 1250.;
    Double_t fdzNl = 400.;

    TVector3 GetNLGlobalPos(TVector3 local);
    TVector3 GetNLLocalPos(TVector3 global);

  ClassDef(STNLDigiTask,1);
};

#endif
