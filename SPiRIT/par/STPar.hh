//---------------------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      STPar reads in parameters for digitization and reconstruction
//      and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//
//----------------------------------------------------------------------

#ifndef _STPAR_H_
#define _STPAR_H_

#include "FairParGenericSet.h"

#include "STGas.hh"

class STPar : public FairParGenericSet
{
  public:
    // Constructors and Destructors
    STPar();
    ~STPar();

    // Operators
    
    // Getters
    STGas *GetGas();

    // Setters

    // Main methods

  private:
    STGas *fGas;

  ClassDef(STPar, 1);
};

#endif
