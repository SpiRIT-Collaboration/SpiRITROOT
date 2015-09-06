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
#include "TString.h"

class STCore;

class STPlot : public TObject
{
  public:
    STPlot();
    STPlot(STCore *core);
    ~STPlot() {};

    void SetSTCore(STCore *core);

    void DrawPadplane(Int_t eventID = -1);
    void ClickPad();
    void DrawLayer(Int_t layer);

    // Setters
    void SetEvent(STRawEvent *anEvent);
    void SetNumTbs(Int_t numTbs);
    void SetPadplaneTitle(TString title);

  private:
    STCore *fCore;
    STMap *fMap;

    void Clear();
    void PreparePadplaneHist();
    void PreparePadCanvas();
    void DrawPad(Int_t row, Int_t layer);
    Bool_t CheckEvent();

    STRawEvent *fEvent;
    Int_t fNumTbs;

    TCanvas *fPadplaneCvs;
    TH2D *fPadplaneHist;
    TString fPadplaneTitle;

    TCanvas *fPadCvs;
    TGraph *fPadGraph[2];

    TH2D *fLayerHist;

  ClassDef(STPlot, 1);
};

#endif
