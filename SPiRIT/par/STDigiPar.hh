//---------------------------------------------------------------------
// Description:
//      STDigiPar reads in parameters and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//----------------------------------------------------------------------

#ifndef _STDIGIPAR_H_
#define _STDIGIPAR_H_

#include "FairParGenericSet.h"

#include "STGas.hh"

class STDigiPar : public FairParGenericSet
{
  public:
    // Constructors and Destructors
    STDigiPar();
    ~STDigiPar();

    // Operators
    
    // Getters
    STGas *GetGas();
    virtual Bool_t getParams(FairParamList *paramList);

    // Setters
    virtual void putParams(FairParamList *paramList);

    // Main methods

  private:
    STGas *fGas;

  ClassDef(STDigiPar, 1);
};

#endif
