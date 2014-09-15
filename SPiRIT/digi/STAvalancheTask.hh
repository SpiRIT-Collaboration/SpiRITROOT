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
#include "TH2D.h"

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
    void SetPersistence(Bool_t val) { fIsPersistent = val; };
    void SetTestMode()              { fTestMode = kTRUE; };
    void SetWriteHistogram()        { fWriteHistogram = kTRUE; };
    
    // Main methods
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    TString fDriftedElectronBranchName;  //!< Name of primary cluster branch name (input)
    TClonesArray* fDriftedElectronArray; //!< drifted electron array (input)
    TClonesArray* fAvalancheArray;            //!< avalanche electron array (output)

    STDigiPar* fPar;                     //!< STDigiPar pointer
    STGas*     fGas;                     //!< STGas pointer
 
    Bool_t fIsPersistent;                //!< If true, save container
    Bool_t fTestMode;                    //!< If true, test mode!
    Bool_t fWriteHistogram;              //!< If true, create histogram electron distribution 

    Int_t fPadPlaneX;
    Int_t fPadPlaneZ;

    void WriteHistogram();
    TH2D *fElectronDistXZ;

  ClassDef(STAvalancheTask, 1);
};

#endif
