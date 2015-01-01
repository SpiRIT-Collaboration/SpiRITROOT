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
  fGas = fPar -> GetGas();

  // wire plane 
  Int_t nWire = 363;
  zCenterWire  = 67.2; // [cm]
  zSpacingWire = 0.4;  // [cm]

  zFirstWire = zCenterWire - (nWire-1)/2 * zSpacingWire;
  zLastWire  = zCenterWire + (nWire-1)/2 * zSpacingWire;

}

Double_t STWireResponse::FindZWire(Double_t z)
{
  Int_t    iWire = floor( (z - zCenterWire + zSpacingWire/2) / zSpacingWire );
  Double_t zWire = iWire * zSpacingWire + zCenterWire;

  if(zWire>zLastWire)  zWire = zLastWire;
  if(zWire<zFirstWire) zWire = zFirstWire;

  return zWire;
}
