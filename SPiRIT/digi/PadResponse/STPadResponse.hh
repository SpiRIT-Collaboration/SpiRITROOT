#pragma once

#include "STDigiPar.hh"
#include "STGas.hh"
#include "STRawEvent.hh"
#include "TH2D.h"
#include "TF1.h"
#include "TMath.h"
#include "TGraph.h"

class STPadResponse
{
  public :
    STPadResponse();
    ~STPadResponse() {};

    void SetRawEvent(STRawEvent* event) { fRawEvent = event; };
    void FillPad(Int_t gain, Double_t x, Double_t t, Int_t zWire);

  private :
    // methods
    void FindPad(Double_t xElectron, 
                    Int_t zWire, 
                    Int_t &row, 
                    Int_t &layer, 
                    Int_t &type);

    // Parameter containers
    STDigiPar* fPar;
    STGas* fGas;

    // Function
    TF1*     fPadResponseFunction1; // par[0] : mean
    Double_t fPadResponseFunction(Double_t *x, Double_t *par);
    TGraph*  fPadResponseIntegralData;

    // STRawEvent
    STRawEvent* fRawEvent;
    TH2D*       fPadPlane;

    //Parameters
    Double_t fTimeMax;
    Int_t    fNTBs;
    Int_t    fXPadPlane;
    Int_t    fZPadPlane;
    Int_t    fBinSizeZ;
    Int_t    fBinSizeX;
    Int_t    fNBinsX;
    Int_t    fNBinsZ;

    Double_t fFillRatio[3][3];

  ClassDef(STPadResponse, 1);
};
