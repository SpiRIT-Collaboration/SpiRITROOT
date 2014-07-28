//---------------------------------------------------------------------
// Description:
//      Pad Response task class header
//
//      Make pad signals
//
//      Input  : STAvalanche
//      Output : STPadSignal
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

#ifndef _STAVALANCHETASK_H_
#define _STAVALANCHETASK_H_

// Fair & Collaborating class headers 
#include "FairTask.h"                 

// ROOT headers
#include "TString.h"
#include "TClonesArray.h"

// SPiRIT-TPC headers
#include "STGas.hh"
#include "STDigiPar.hh"

class STPadResponse : public FairTask
{
  public:
    // Constructor and Destructor
    STPadResponse();
    ~STPadResponse();
    
    // Operators
    // Getters
    // Setters
    void SetPersistence(Bool_t val) { fIsPersistent = val; };
    
    // Main methods
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    TString fAvalancheBranchName;        //!< Name of primary cluster branch name (input)
    TClonesArray* fAvalancheArray;       //!< avalanche electron array (input)
    TClonesArray* fPadSignalArray;       //!< drifted electron array (output)

    STDigiPar* fPar;                     //!< STDigiPar pointer
    STGas*     fGas;                     //!< STGas pointer
 
    Bool_t fIsPersistent;                //!< If true, save container

  ClassDef(STPadResponse, 1);
};

#endif
