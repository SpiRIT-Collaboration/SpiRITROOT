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
#include "TMarker.h"

class STCore;

class STPlot : public TObject
{
  public:
    STPlot();
    STPlot(STCore *core);
    ~STPlot() {};

    enum EClickEvent { kNothing, kDrawPad, kDrawLayer };

    void SetSTCore(STCore *core);

    void DrawPadplane(Int_t eventID = -1);
    void ClickPad();

    void DrawSideview(Int_t eventID = -1);
    void ClickLayer();

    TCanvas *GetPadplaneCanvas();
    TCanvas *GetSideviewCanvas();

    // Setters
    void SetEvent(STRawEvent *anEvent);
    void SetNumTbs(Int_t numTbs);

    void SetPadplaneTitle(TString title);
    void SetSideviewTitle(TString title);

  private:
    STCore *fCore;
    STMap *fMap;

    void Clear();
    void PreparePadplaneHist(EClickEvent mode = kNothing);
    void PreparePadCanvas();
    void DrawPad(Int_t row, Int_t layer);

    void PrepareSideviewHist(EClickEvent mode = kNothing);
    void PrepareLayerHist();
    void DrawLayer(Int_t layer);

    Bool_t CheckEvent();

    STRawEvent *fEvent;
    Int_t fNumTbs;

    TCanvas *fPadplaneCvs;
    TH2D *fPadplaneHist;
    TString fPadplaneTitle;

    TCanvas *fSideviewCvs;
    TH2D *fSideviewHist;
    TString fSideviewTitle;

    TCanvas *fPadCvs;
    TGraph *fPadGraph[2];

    TCanvas *fLayerCvs;;
    TH2D *fLayerHist;
    TString fLayerHistTitle;

    TMarker *fMarkerPadplane;
    TMarker *fMarkerSideview;

  ClassDef(STPlot, 1);
};

#endif
