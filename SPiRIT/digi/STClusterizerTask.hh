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
//
//----------------------------------------------------------------------

#ifndef _STCLUSTERIZERTASK_H_
#define _STCLUSTERIZERTASK_H_

// ROOT headers
#include "TString.h"
#include "TClonesArray.h"

// SPiRIT-TPC headers
#include "STGas.hh"
#include "STPar.hh"

class STClusterizerTask : public FairTask
{
  public:
    // Constructor and Destructor
    STClusterizerTask();
    ~STClusterizerTask();
    
    // Operators
    // Getters
    // Setters
    
    // Main methods
    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

  private:
    TString fMCPointBranchName;
    TCloneArray *fMCPointArray;
    TCloneArray *fPrimaryClusterArray;

    const STGas fGas;
    STPar *fPar;

    Bool_t fIsPersistant;

  ClassDef(STClusterizerTask, 1);
};

#endif
