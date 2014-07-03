//---------------------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      STClusterizer reads in MCPoints and produces primary electrons
//
// Author List:
//      Sebastian Neubert    TUM            (original author)
//
// Rewritten for SPiRIT-TPC:
//      Genie Jhang     Korea Univ.
//      JungWoo Lee     Korea Univ.
//
//----------------------------------------------------------------------

#ifndef _STCLUSTERIZERTASK_H_
#define _STCLUSTERIZERTASK_H_

// ROOT headers
#include "TString.h"
#include "TClonesArray.h"

// SPiRIT-TPC headers
#include "STGas.hh"
#include "STDigiPar.hh"

class STClusterizerTask : public FairTask
{
  public:
    // Constructor and Destructor
    STClusterizerTask();
    ~STClusterizerTask();
    
    // Operators
    // Getters
    // Setters
    void SetPersistant(Bool_t val)             { fIsPersistant = val }
    void SetSimpleChargeConversion(Bool_t val) { fIsSimpleChargeConversion = val }
    
    // Main methods
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    // variables
    TString fMCPointBranchName;        //!< Name of MC data branch
    TCloneArray *fMCPointArray;        //!< Point array for MC data
    TCloneArray *fPrimaryClusterArray; //!< Primary cluster array

    const STGas *fGas;                 //!< STGas pointer
    STDigiPar *fPar;                   //!< STDigiPar pointer
    Float_t fFirstIonizationPotential; //!< First ionization potential

    Bool_t fIsPersistant;              //!< If true, save container
    Bool_t fIsSimpleChargeConversion;  //!< If true, do simple charge conversion

    // methods
    void ChargeConversion();

  ClassDef(STClusterizerTask, 1);
};

#endif
