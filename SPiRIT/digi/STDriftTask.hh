//---------------------------------------------------------------------
// Description:
//      Drift task class header
//
//      STDriftTask apply diffusions and velocity of electron and
//      produces electrons drifted through the gas
//
//      Input  : STPrimaryCluster
//      Output : STDrifedElectron
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

#ifndef _STDRIFTTASK_H_
#define _STDRIFTTASK_H_

// Fair & Collaborating class headers 
#include "FairTask.h"                 

// ROOT headers
#include "TString.h"
#include "TClonesArray.h"
#include "TH2D.h"

// SPiRIT-TPC headers
#include "STGas.hh"
#include "STDigiPar.hh"

class STDriftTask : public FairTask
{
  public:
    // Constructor and Destructor
    STDriftTask();
    ~STDriftTask();
    
    // Operators
    // Getters
    // Setters
    void SetPersistence(Bool_t val) { fIsPersistent = val; };
    void SetTestMode()              { fTestMode = kTRUE; };
    void SetWriteHistogram()        { fWriteHistogram = kTRUE; };
    
    // Main methods
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    TString fPrimaryClusterBranchName;   //!< Name of primary cluster branch name (input)
    TClonesArray* fPrimaryClusterArray;  //!< primary cluster array (input)
    TClonesArray* fDriftedElectronArray; //!< drifted electron array (output)

    STDigiPar* fPar;                     //!< STDigiPar pointer
    STGas*     fGas;                     //!< STGas pointer
 
    Bool_t fIsPersistent;                //!< If true, save container
    Bool_t fTestMode;                    //!< If true, test mode!
    Bool_t fWriteHistogram;              //!< If true, create histogram electron distribution 

    Int_t fPadPlaneX;
    Int_t fPadPlaneZ;

    void WriteHistogram();
    TH2D *fElectronDistXZ;
    TH2D *fElectronDistXZRaw;

  ClassDef(STDriftTask, 1);
};

#endif
