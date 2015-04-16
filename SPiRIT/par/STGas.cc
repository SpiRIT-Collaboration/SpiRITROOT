//---------------------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      STGas reads in gas property file and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//      JungWoo Lee     Korea Univ.
//
//----------------------------------------------------------------------

// This class header
#include "STGas.hh"

// ROOT class headers
#include "TRandom.h"

// C/C++ class headers
#include <stdio.h>
#include <string.h>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>
#include <iostream>

using namespace std;

ClassImp(STGas)

STGas::STGas(TString GasFileName)
: fGasFileName(GasFileName)
{
  InitializeParameters();
}

void STGas::InitializeParameters()
{
  ifstream gasFile(fGasFileName.Data(), std::fstream::in);
  if(!gasFile.is_open()) cerr << "Gas file " << fGasFileName.Data() << " not found!!" << endl;

  string line;
  string data;
  string name;
  string format;
  string val;

  while(getline(gasFile,line)) {
    istringstream ss_line(line);
    if(ss_line >> data >> val && data[0]!='#' && data[0]!='[') {
      name   = data.substr(0,data.find(":"));
      //name   = strtok(data.c_str(),":");
      //format = strtok(NULL,":");
      if(name=="EIonizeP10")         fEIonize            = atof(val.c_str());
      if(name=="DriftVelocity")      fDriftVelocity      = atof(val.c_str());
      if(name=="CoefAttachment")     fCoefAttachment     = atof(val.c_str());
      if(name=="CoefDiffusionLong")  fCoefDiffusionLong  = atof(val.c_str());
      if(name=="CoefDiffusionTrans") fCoefDiffusionTrans = atof(val.c_str());
      if(name=="Gain")               fGain               = atoi(val.c_str());
    }
  }
}

STGas::~STGas()
{
}

void STGas::operator=(const STGas& GasToCopy) 
{ 
  fEIonize            = GasToCopy.fEIonize; 
  fDriftVelocity      = GasToCopy.fDriftVelocity; 
  fCoefAttachment     = GasToCopy.fCoefAttachment; 
  fCoefDiffusionLong  = GasToCopy.fCoefDiffusionLong; 
  fCoefDiffusionTrans = GasToCopy.fCoefDiffusionTrans; 
  fGain               = GasToCopy.fGain; 
}

Double_t STGas::GetEIonize()            { return fEIonize; }
Double_t STGas::GetDriftVelocity()      { return fDriftVelocity; }
Double_t STGas::GetCoefAttachment()     { return fCoefAttachment; }
Double_t STGas::GetCoefDiffusionLong()  { return fCoefDiffusionLong; }
Double_t STGas::GetCoefDiffusionTrans() { return fCoefDiffusionTrans; }
   Int_t STGas::GetGain()               { return fGain; }
  UInt_t STGas::GetRandomCS()           
{
  UInt_t CS = (UInt_t)(gRandom -> Gaus(50,20));
  if(CS<=0) CS=1;
  return CS;
}
