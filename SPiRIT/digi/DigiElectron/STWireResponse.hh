#ifndef STWIRERESPONSE_HH
#define STWIRERESPONSE_HH

#include "STGas.hh"

#include "TLorentzVector.h"
#include "TH2D.h"
#include "TF2.h"

class STWireResponse
{
  public :
    STWireResponse(STGas* gas);
    ~STWireResponse() {};

    Double_t FindZWire(Double_t z);
    Int_t    FillPad(Double_t x);
    void     WriteHistogram();

  private :
    TH2D* fPadPlane;
    TF2*  fWPField;

    STGas* fGas;

    Double_t fGain;

    Int_t    nWire;
    Double_t zCenterWire;
    Double_t zSpacingWire;
    Double_t zFirstWire;
    Double_t zLastWire;
    Int_t    iWire;
    Double_t zWire;

    // Electron to Signal
    Double_t gain;

  ClassDef(STWireResponse, 1);
};

#endif
