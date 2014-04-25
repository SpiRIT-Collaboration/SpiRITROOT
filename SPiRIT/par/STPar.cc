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

#include "STPar.hh"

ClassImp(STPar)

STPar::STPar()
:FairParGenericSet("STPar", "SPiRIT Parameter Container", "")
{
}

STPar::~STPar()
{
}

STGas *STPar::GetGas()
{
  return fGas;
}
