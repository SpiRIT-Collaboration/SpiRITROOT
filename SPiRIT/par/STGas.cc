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

Double_t
STGas::GetEIonize()
{ 
  return fEIonize;
}

Double_t
STGas::GetFirstIonizationPotential()
{
  return fFirstIonizationPotential;
}

Double_t
STGas::GetRandomCS(Double_t val)
{
}
