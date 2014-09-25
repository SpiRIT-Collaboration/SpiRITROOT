#ifndef STWIRERESPONSE_HH
#define STWIRERESPONSE_HH

#include "STGas.hh"
#include "STRawEvent.hh"
#include "STPad.hh"

#include "TLorentzVector.h"
#include "TH2D.h"
#include "TF2.h"

class STWireResponse
{
  public :
    STWireResponse(STGas* gas);
    ~STWireResponse() {};

    Double_t FindZWire(Double_t z);
    Int_t    FillPad(Double_t x, Double_t t);
    void     WriteHistogram();

    void     SetRawEvent(STRawEvent* event, Int_t nTBsP) 
             { fEvent = event; nTBs = nTBsP; };

  private :
    TH2D* fPadPlane;
    TF2*  fWPField;

    STGas* fGas;

    Double_t maxTime;

    Double_t fGain;
    Double_t zCenterWire;
    Double_t zSpacingWire;
    Double_t zFirstWire;
    Double_t zLastWire;
    Double_t zWire;

    Double_t xPadPlane;
    Double_t zPadPlane;

    Double_t binSizeZ;
    Double_t binSizeX;

    // Electron to Signal
    Double_t gain;

    // Function
    Double_t WPField(Double_t *x, Double_t *par);

    // STRawEvent
    STRawEvent* fEvent;
    STPad*      pad;
    Int_t       nTBs;

  ClassDef(STWireResponse, 1);
};

#endif
