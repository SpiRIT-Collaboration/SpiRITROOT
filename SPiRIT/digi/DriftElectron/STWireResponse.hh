#ifndef STWIRERESPONSE_HH
#define STWIRERESPONSE_HH

#include "STDigiPar.hh"
#include "STGas.hh"

#include "TLorentzVector.h"

class STWireResponse
{
  public :
    STWireResponse();
    ~STWireResponse() {};

    Int_t FindZWire(Double_t z);

  private :

    STDigiPar* fPar;

    Int_t zCenterWire;
    Int_t  zSpacingWire;
    Int_t zFirstWire;
    Int_t zLastWire;

  ClassDef(STWireResponse, 1);
};

#endif
