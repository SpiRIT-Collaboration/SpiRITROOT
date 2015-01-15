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

    void Init();
    void FillPad(Int_t gain, Double_t x, Double_t t, Int_t zWire);
    void CloneRawEvent(STRawEvent *rawEvent);

  private :
    // methods
    void InitializeRawEventDummy();
    void FindPad(Double_t xElectron, 
                    Int_t zWire, 
                    Int_t &row, 
                    Int_t &layer, 
                    Int_t &type);

    // Parameter containers
    STDigiPar* fPar;
    STGas* fGas;

    // Function
    TGraph*  fPadResponseIntegralData;

    // STRawEvent
    STRawEvent* fRawEventDummy;
    Bool_t fIsActivePad[108*112];

    //Parameters
    Double_t fTimeMax;
    Int_t    fNTBs;
    Int_t    fXPadPlane;
    Int_t    fZPadPlane;
    Int_t    fBinSizeLayer;
    Int_t    fBinSizeRow;
    Int_t    fNRows;
    Int_t    fNLayers;

    Double_t fFillRatio[3][3];

  ClassDef(STPadResponse, 1);
};
