// =================================================
//  STPlot Class
//
//  Description:
//    Plot event data
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 24
// =================================================

#ifndef STPLOT_H
#define STPLOT_H

#include "TROOT.h"

#include "STCore.hh"
#include "STRawEvent.hh"
#include "STPad.hh"

#include "TCanvas.h"
#include "TH2D.h"
#include "TH1D.h"
#include "TGraph.h"

class STCore;

class STPlot : public TObject
{
  public:
    STPlot();
    STPlot(STCore *core);
    ~STPlot() {};

    void DrawPadplane();
    void DrawPad(Int_t row, Int_t layer);
    void DrawLayer(Int_t layer);

    // Setters
    void SetEvent(STRawEvent *anEvent);
    void SetNumTbs(Int_t numTbs);

    // Getters

  private:
    STCore *fCore;

    void Clear();
    void PreparePadplaneHist();
    void PreparePadCanvas();
    Bool_t CheckEvent();

    STRawEvent *fEvent;
    Int_t fNumTbs;

    TCanvas *padplaneCvs;
    TH2D *padplaneHist;

    TCanvas *padCvs;
    TGraph *padGraph[2];

    TH2D *layerHist;

  ClassDef(STPlot, 1);
};

#endif
