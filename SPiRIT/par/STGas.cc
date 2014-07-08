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

// This class header
#include "STGas.hh"

// ROOT class headers
#include "TRandom.h"

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

Double_t STGas::GetEIonize()
{ 
  return fEIonize;
}

Double_t STGas::GetDriftVelocity()
{ 
  return fDriftVelocity;
}

Double_t STGas::GetCoefAttachment()
{ 
  return fCoefAttachment;
}

Double_t STGas::GetCoefDiffusion()
{ 
  return fCoefDiffusion;
}

UInt_t STGas::GetRandomCS()
{
  UInt_t CS = (UInt_t)(gRandom -> Gaus(50,20));
  if(CS==0) CS=1;
  return CS;
}
