/**
 * @brief Calculate z-position of wire
 *
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STWireResponse.hh"
#include "FairRootManager.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include <iostream>
#include "TRandom.h"

using std::cout;
using std::endl;

ClassImp(STWireResponse);

STWireResponse::STWireResponse()
{
  FairRunAna* ana = FairRunAna::Instance();
  FairRuntimeDb* rtdb = ana->GetRuntimeDb();
  fPar = (STDigiPar*) rtdb->getContainer("STDigiPar");

  Int_t nWire = 363; // number of wires
  Int_t zPadPlane = fPar -> GetPadPlaneZ();
  fZCenterWire = zPadPlane/2;
  fZSpacingWire = 4;

  fZFirstWire = fZCenterWire - (nWire-1)/2 * fZSpacingWire;
  fZLastWire  = fZCenterWire + (nWire-1)/2 * fZSpacingWire;
}

Int_t STWireResponse::FindZWire(Double_t z)
{
  Int_t iWire = floor( (z - fZCenterWire + fZSpacingWire/2) / fZSpacingWire );
  Int_t zWire = iWire * fZSpacingWire + fZCenterWire;

  if(zWire>fZLastWire)  zWire = fZLastWire;
  if(zWire<fZFirstWire) zWire = fZFirstWire;

  return zWire;
}
