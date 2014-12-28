#pragma once

#include "STDigiPar.hh"
#include "STGas.hh"
#include "STRawEvent.hh"
#include "TH2D.h"
#include "TF2.h"
#include "TMath.h"

class STPadResponse
{
  public :
    STPadResponse();
    ~STPadResponse() {};

    void  SetRawEvent(STRawEvent* event) { fRawEvent = event; };
    Int_t FillPad(Double_t x, Double_t t, Double_t zWire);
    void  WriteHistogram();

  private :
    // Parameter containers
    STDigiPar* fPar;
    STGas* fGas;

    // Function
    TF2*     fWPField;
    Double_t WPField(Double_t *x, Double_t *par);

    // STRawEvent
    STRawEvent* fRawEvent;
    STPad*      pad;
    TH2D*       fPadPlane;

    //Parameters
    Int_t    fGain;
    Int_t    fNTBs;
    Double_t maxTime;
    Double_t xPadPlane;
    Double_t zPadPlane;
    Double_t binSizeZ;
    Double_t binSizeX;

  ClassDef(STPadResponse, 1);
};
