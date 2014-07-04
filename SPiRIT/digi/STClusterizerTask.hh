//---------------------------------------------------------------------
// Description:
//      Clusterizer task class header
//
//      STClusterizer reads in MCPoints and produces primary clusters
//      Input  : STMC
//      Output : STPrimaryCluster
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

#ifndef _STCLUSTERIZERTASK_H_
#define _STCLUSTERIZERTASK_H_

// Fair & Collaborating class headers
#include "FairTask.h"

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
    void SetPersistant(Bool_t val) { fIsPersistent = val; };
    
    // Main methods
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    // variables
    TString fMCPointBranchName;        //!< Name of MC data branch
    TClonesArray *fMCPointArray;        //!< Point array for MC data
    TClonesArray *fPrimaryClusterArray; //!< Primary cluster array

    //const STGas *fGas;
    STGas *fGas;                       //!< STGas pointer
    STDigiPar *fPar;                   //!< STDigiPar pointer

    Bool_t fIsPersistent;              //!< If true, save container

  ClassDef(STClusterizerTask, 1);
};

#endif
