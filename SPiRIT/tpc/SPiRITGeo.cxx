#include "SPiRITGeo.h"
#include "FairGeoNode.h"

ClassImp(SPiRITGeo)

// -----   Default constructor   -------------------------------------------
SPiRITGeo::SPiRITGeo()
  : FairGeoSet()
{
  // Constructor
  // fName has to be the name used in the geometry for all volumes.
  // If there is a mismatch the geometry cannot be build.
  fName="newdetector";
  maxSectors=0;
  maxModules=10;
}

// -------------------------------------------------------------------------

const char* SPiRITGeo::getModuleName(Int_t m)
{
  /** Returns the module name of SPiRIT number m
      Setting SPiRIT here means that all modules names in the
      ASCII file should start with SPiRIT otherwise they will
      not be constructed
  */
  sprintf(modName,"SPiRIT%i",m+1);
  return modName;
}

const char* SPiRITGeo::getEleName(Int_t m)
{
  /** Returns the element name of Det number m */
  sprintf(eleName,"SPiRIT%i",m+1);
  return eleName;
}
