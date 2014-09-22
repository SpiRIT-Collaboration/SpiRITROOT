//---------------------------------------------------------------------
// Description:
//      DigiElectron task class header
//
//      STDigiElectron reads in MCPoints and produces primary clusters
//
//      Input  : STMC
//      Output : STDigitizedElectron
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

#ifndef _STDIGIELECTRONTASK_H_
#define _STDIGIELECTRONTASK_H_

// Fair & Collaborating class headers
#include "FairTask.h"

// ROOT headers
#include "TString.h"
#include "TClonesArray.h"

// SPiRIT-TPC headers
#include "STGas.hh"
#include "STDigiPar.hh"
#include "STMCPoint.hh"
#include "STDriftElectron.hh"
#include "STWireResponse.hh"

class STDigiElectronTask : public FairTask
{
  public:
    // Constructor and Destructor
    STDigiElectronTask();
    ~STDigiElectronTask();
    
    // Operators
    // Getters
    // Setters
    void SetPersistence(Bool_t val) { fIsPersistent = val; };
    
    // Main methods
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    // variables
    TString fMCPointBranchName;            /// Name of MC data branch
    TClonesArray *fMCPointArray;           /// Point array for MC data
    TClonesArray *fDigitizedElectronArray; /// Primary cluster array

    STDriftElectron* driftElectron;
    STWireResponse*  wireResponse;

    STMCPoint* MCPoint; /// STMCPoint
    STDigiPar* fPar;    /// STDigiPar pointer
    STGas*     fGas;    /// STGas pointer

    Double_t EIonize;   /// ionization energy


    Bool_t fIsPersistent; /// If true, save container

  ClassDef(STDigiElectronTask, 1);
};

#endif
