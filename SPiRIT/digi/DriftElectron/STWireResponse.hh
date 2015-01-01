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

    Double_t FindZWire(Double_t z);

  private :

    STDigiPar* fPar;
    STGas* fGas;

    Double_t zCenterWire;
    Double_t zSpacingWire;
    Double_t zFirstWire;
    Double_t zLastWire;

  ClassDef(STWireResponse, 1);
};

#endif
