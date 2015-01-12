#include "TRandom.h"

#include "STWireResponse.hh"
#include "FairRootManager.h"
#include "iostream"

#include "FairRunAna.h"
#include "FairRuntimeDb.h"

using std::cout;
using std::endl;

ClassImp(STWireResponse);

STWireResponse::STWireResponse()
{
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");

  // wire plane 
  Int_t nWire = 363;
  Int_t zPadPlane = fPar -> GetPadPlaneZ();
  zCenterWire  = zPadPlane/2;
  zSpacingWire = 4;  // [mm]

  zFirstWire = zCenterWire - (nWire-1)/2 * zSpacingWire;
  zLastWire  = zCenterWire + (nWire-1)/2 * zSpacingWire;
}

Int_t STWireResponse::FindZWire(Double_t z)
{
  z *= 10; // [cm] to [mm]

  Int_t iWire = floor( (z - zCenterWire + zSpacingWire/2) / zSpacingWire );
  Int_t zWire = iWire * zSpacingWire + zCenterWire;

  if(zWire>zLastWire)  zWire = zLastWire;
  if(zWire<zFirstWire) zWire = zFirstWire;

  return zWire;
}
