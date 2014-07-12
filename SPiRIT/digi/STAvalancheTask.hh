//---------------------------------------------------------------------
// Description:
//      Avalanche task class header
//
//      Make avalanch electrons drifted through wire plane.
//
//      Input  : STDrifedElectron
//      Output : STAvalanche
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

class STAvalancheTask : public FairTask
{
  public:
    // Constructor and Destructor
    STAvalancheTask();
    ~STAvalancheTask();
    
    // Operators
    // Getters
    // Setters
    
    // Main methods
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    TString fDriftedElectronBranchName;  //!< Name of primary cluster branch name (input)
    TClonesArray* fDriftedElectronArray; //!< drifted electron array (input)
    TClonesArray* fAvalanche;            //!< avalanche electron array (output)

    STDigiPar* fPar;                     //!< STDigiPar pointer
    STGas*     fGas;                     //!< STGas pointer
 
    Bool_t fIsPersistent;                //!< If true, save container

  ClassDef(STAvalancheTask, 1);
};

#endif
