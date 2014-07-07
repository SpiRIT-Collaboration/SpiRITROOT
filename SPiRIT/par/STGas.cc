//---------------------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      STGas reads in gas property file and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//
//----------------------------------------------------------------------

#include "STGas.hh"

ClassImp(STGas)

STGas::STGas()
{
}

STGas::~STGas()
{
}

void STGas::operator=(const STGas& GasToCopy)
{
  fEIonize = GasToCopy.fEIonize;
}

Double_t
STGas::GetEIonize()
{ 
  return fEIonize;
}

Double_t
STGas::GetRandomCS(Double_t val)
{
}
