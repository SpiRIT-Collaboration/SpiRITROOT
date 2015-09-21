// =================================================
//  STPlot Class
//
//  Description:
//    Plot event data
//
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 24
// =================================================

#include "STPlot.hh"
#include "STRawEvent.hh"
#include "STPad.hh"
#include "STStatic.hh"
#include "STMap.hh"

#include "TCanvas.h"
#include "TStyle.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLatex.h"
#include "TGaxis.h"

#include <iostream>

ClassImp(STPlot)

STPlot::STPlot()
{
  Clear();
}

STPlot::STPlot(STCore *core)
{
  Clear();

  SetSTCore(core);
}

void STPlot::SetSTCore(STCore *core)
{
  fCore = core;
  fMap = core -> GetSTMap();
  SetNumTbs(fCore -> GetNumTbs());
}

void STPlot::Clear()
{
  fCore = NULL;
  fMap = NULL;

  fEvent = NULL;
  fNumTbs = 512;

  fPadplaneCvs = NULL;
  fPadplaneHist = NULL;
  fPadplaneTitle = "Event display - Event ID: %d - Top view";

  fSideviewCvs = NULL;
  fSideviewHist = NULL;
  fSideviewTitle = "Event display - Event ID: %d - Beamright view";

  fPadCvs = NULL;
  fPadGraph[0] = NULL;
  fPadGraph[1] = NULL;

  fLayerCvs = NULL;
  fLayerHist = NULL;

  fMarkerPadplane = new TMarker();
  fMarkerSideview = new TMarker();
}

Bool_t STPlot::CheckEvent()
{
  if (fEvent == NULL) {
    std::cerr << "== [STPlot] Event is not set!" << std::endl;
    return 1;
  }

  return 0;
}

// Setters
void STPlot::SetEvent(STRawEvent *anEvent)
{
  fEvent = anEvent;

  std::cerr << "== [STPlot] Default number of time buckets is 512." << std::endl;
}

void STPlot::SetNumTbs(Int_t numTbs)
{
  fNumTbs = numTbs;
}

void STPlot::SetPadplaneTitle(TString title)
{
  fPadplaneTitle = title;
}

void STPlot::SetSideviewTitle(TString title)
{
  fSideviewTitle = title;
}

void STPlot::DrawPadplane(Int_t eventID)
{
  if (fCore != NULL) {
    if (eventID != fCore -> GetEventID())
      fEvent = fCore -> GetRawEvent(eventID);
  }
  
  if (CheckEvent())
    return;

  if (fPadplaneHist)
    fPadplaneHist -> Reset();
  else
    PreparePadplaneHist(kDrawPad);

  Int_t numPads = fEvent -> GetNumPads();
  Double_t max = 0;

  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad *aPad = fEvent -> GetPad(iPad);

    Double_t *adc = aPad -> GetADC();

    Double_t maxADC = 0;
    for (Int_t i = 0; i < fNumTbs; i++) {
      if (maxADC < aPad -> GetADC(i))
        maxADC = aPad -> GetADC(i);
    }

    fPadplaneHist -> SetBinContent(aPad -> GetLayer() + 1, aPad -> GetRow() + 1, maxADC);
    if (maxADC > max) max = maxADC;
  }

  fPadplaneHist -> SetTitle(Form(Form("%s", fPadplaneTitle.Data()), fEvent -> GetEventID()));
  fPadplaneHist -> GetZaxis() -> SetRangeUser(0.1, fPadplaneHist -> GetBinContent(fPadplaneHist -> GetMaximumBin()) + 100);
  fPadplaneHist -> SetContour(50);

  fPadplaneCvs -> Modified();
  fPadplaneCvs -> Update();
}

void STPlot::ClickPad()
{
  TObject *select = gPad -> GetCanvas() -> GetClickSelected();

  if (select == NULL ||
      (!(select -> InheritsFrom(TH2::Class())) && !(select -> InheritsFrom(TGraph::Class()))))
    return;

  Int_t xEvent = gPad -> GetEventX();
  Int_t yEvent = gPad -> GetEventY();

  Double_t xAbs = gPad -> AbsPixeltoX(xEvent);
  Double_t yAbs = gPad -> AbsPixeltoY(yEvent);

  Double_t xOnClick = gPad -> PadtoX(xAbs);
  Double_t yOnClick = gPad -> PadtoY(yAbs);

  Int_t bin = fPadplaneHist -> FindBin(xOnClick, yOnClick);

  gPad -> SetUniqueID(bin);
  gPad -> GetCanvas() -> SetClickSelected(NULL);

  Int_t row = (yOnClick + 432)/8;
  Int_t layer = xOnClick/12;

  Double_t padCenterX = (row + 0.5)*8. - 432;
  Double_t padCenterZ = (layer + 0.5)*12.;

  fMarkerPadplane -> SetMarkerStyle(4);
  fMarkerPadplane -> SetX(padCenterZ);
  fMarkerPadplane -> SetY(padCenterX);
  fMarkerPadplane -> SetMarkerSize(2);
  fMarkerPadplane -> Draw("same");

  for (Int_t iCvs = 0; iCvs < gROOT -> GetListOfCanvases() -> GetEntries(); iCvs++) {
    TCanvas *cvs = (TCanvas *) gROOT -> GetListOfCanvases() -> At(iCvs);
    if (TString(cvs -> GetName()).Contains("fPadplaneCvs")) {
      cvs  -> Modified();
      cvs -> Update();
    }
  }

  DrawPad(row, layer);
}

void STPlot::DrawSideview(Int_t eventID)
{
  if (fCore != NULL) {
    if (eventID != fCore -> GetEventID())
      fEvent = fCore -> GetRawEvent(eventID);
  }
  
  if (CheckEvent())
    return;

  if (fSideviewHist)
    fSideviewHist -> Reset();
  else
    PrepareSideviewHist(kDrawLayer);

  Int_t numPads = fEvent -> GetNumPads();

  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad *aPad = fEvent -> GetPad(iPad);

    Double_t *adc = aPad -> GetADC();

    for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
      Double_t prevADC = fSideviewHist -> GetBinContent(aPad -> GetLayer() + 1, fNumTbs - iTb);
      Double_t currADC = aPad -> GetADC(iTb);
      if (prevADC < currADC)
        fSideviewHist -> SetBinContent(aPad -> GetLayer() + 1, fNumTbs - iTb, currADC);
    }
  }

  fSideviewHist -> SetTitle(Form(Form("%s", fSideviewTitle.Data()), fEvent -> GetEventID()));
  fSideviewHist -> GetYaxis() -> SetRangeUser(0, -fNumTbs);
  fSideviewHist -> GetZaxis() -> SetRangeUser(fSideviewHist -> GetBinContent(fSideviewHist -> GetMinimumBin()) - 10, fSideviewHist -> GetBinContent(fSideviewHist -> GetMaximumBin()) + 100);
  fSideviewHist -> SetContour(50);

  fSideviewCvs -> Modified();
  fSideviewCvs -> Update();
}

void STPlot::ClickLayer()
{
  TObject *select = gPad -> GetCanvas() -> GetClickSelected();

  if (select == NULL ||
      (!(select -> InheritsFrom(TH2::Class())) && !(select -> InheritsFrom(TGraph::Class()))))
    return;

  Int_t xEvent = gPad -> GetEventX();
  Int_t yEvent = gPad -> GetEventY();

  Double_t xAbs = gPad -> AbsPixeltoX(xEvent);
  Double_t yAbs = gPad -> AbsPixeltoY(yEvent);

  Double_t xOnClick = gPad -> PadtoX(xAbs);
  Double_t yOnClick = gPad -> PadtoY(yAbs);

  Int_t bin = fSideviewHist -> FindBin(xOnClick, yOnClick);

  gPad -> SetUniqueID(bin);
  gPad -> GetCanvas() -> SetClickSelected(NULL);

  Int_t layer = xOnClick/12;

  Double_t layerCenterZ = (layer + 0.5)*12.;

  fMarkerSideview -> SetMarkerStyle(23);
  fMarkerSideview -> SetX(layerCenterZ);
  fMarkerSideview -> SetY(8);
  fMarkerSideview -> SetMarkerColor(2);
  fMarkerSideview -> SetMarkerSize(2);
  fMarkerSideview -> Draw("same");

  for (Int_t iCvs = 0; iCvs < gROOT -> GetListOfCanvases() -> GetEntries(); iCvs++) {
    TCanvas *cvs = (TCanvas *) gROOT -> GetListOfCanvases() -> At(iCvs);
    if (TString(cvs -> GetName()).Contains("fSideviewCvs")) {
      cvs  -> Modified();
      cvs -> Update();
    }
  }

  DrawLayer(layer);
}

void STPlot::DrawPad(Int_t row, Int_t layer)
{
  if (fPadCvs == NULL)
    PreparePadCanvas();
  else {
    fPadGraph[0] -> Set(0);
    fPadGraph[1] -> Set(0);
  }

  STPad *pad = fEvent -> GetPad(row, layer);
  if (!pad) {
    std::cerr << "== [STPlot] There's no pad (" << row << ", " << layer << ")!" << std::endl;
    return;
  }

  Int_t *tempRawAdc = pad -> GetRawADC();
  Double_t tb[512] = {0};
  Double_t rawAdc[512] = {0};
  for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
    tb[iTb] = iTb;
    rawAdc[iTb] = tempRawAdc[iTb];
  }

  fPadGraph[0] = new TGraph(fNumTbs, tb, rawAdc);
  fPadGraph[0] -> SetTitle(Form("Raw ADC - (row=%d, layer=%d)", row, layer));
  fPadGraph[0] -> SetLineColor(2);
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetTitle("Time bucket");
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> CenterTitle();
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetTitleOffset(1.05);
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetTitleSize(0.05);
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetLabelSize(0.05);
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetLimits(-10, fNumTbs + 10);
  fPadGraph[0] -> GetHistogram() -> GetXaxis() -> SetRangeUser(-10, fNumTbs + 10);
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> SetTitle("ADC");
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> CenterTitle();
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> SetTitleOffset(1.75);
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> SetTitleSize(0.05);
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> SetLabelSize(0.05);
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> SetLimits(-10, 4306);
  fPadGraph[0] -> GetHistogram() -> GetYaxis() -> SetRangeUser(-10, 4306);

  fPadCvs -> cd(1);
  fPadGraph[0] -> Draw("AL");
  if (fMap != NULL) {
    Int_t uaIdx, coboIdx, asadIdx, agetIdx, chIdx;
    fMap -> GetMapData(row, layer, uaIdx, coboIdx, asadIdx, agetIdx, chIdx);

    TLatex *text = new TLatex();
    text -> DrawLatexNDC(0.27, 0.85, Form("UA%03d, C%02d, As%d, Ag%d, Ch%d", uaIdx, coboIdx, asadIdx, agetIdx, chIdx));
  }

  Double_t *adc = pad -> GetADC();
  fPadGraph[1] = new TGraph(fNumTbs, tb, adc);
  fPadGraph[1] -> SetTitle(Form("ADC(FPN subtracted) - (row=%d, layer=%d)", row, layer));
  fPadGraph[1] -> SetLineColor(2);
  fPadGraph[1] -> GetHistogram() -> GetXaxis() -> SetTitle("Time bucket");
  fPadGraph[1] -> GetHistogram() -> GetXaxis() -> CenterTitle();
  fPadGraph[1] -> GetHistogram() -> GetXaxis() -> SetTitleOffset(1.05);
  fPadGraph[1] -> GetHistogram() -> GetXaxis() -> SetTitleSize(0.05);
  fPadGraph[1] -> GetHistogram() -> GetXaxis() -> SetLabelSize(0.05);
  fPadGraph[1] -> GetHistogram() -> GetXaxis() -> SetLimits(-10, fNumTbs + 10);
  fPadGraph[1] -> GetHistogram() -> GetXaxis() -> SetRangeUser(-10, fNumTbs + 10);
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> SetTitle("ADC");
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> CenterTitle();
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> SetTitleOffset(1.75);
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> SetTitleSize(0.05);
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> SetLabelSize(0.05);
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> SetLimits(-10, 4106);
  fPadGraph[1] -> GetHistogram() -> GetYaxis() -> SetRangeUser(-10, 4106);

  fPadCvs -> cd(2);
  fPadGraph[1] -> Draw("AL");

  fPadCvs -> Modified();
  fPadCvs -> Update();
}

void STPlot::DrawLayer(Int_t layerNo)
{
  if (fLayerCvs == NULL)
    PrepareLayerHist();
  else
    fLayerHist -> Reset();

  Int_t numPads = fEvent -> GetNumPads();

  for (Int_t iPad = 0; iPad < numPads; iPad++) {
    STPad *aPad = fEvent -> GetPad(iPad);
    
    if (aPad -> GetLayer() != layerNo)
      continue;

    Double_t *adc = aPad -> GetADC();

    for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
      fLayerHist -> SetBinContent(108 - aPad -> GetRow(), fNumTbs - iTb, aPad -> GetADC(iTb));
  }

  fLayerHist -> SetTitle(Form("Event ID: %d - Layer: %d - Upstream view", fEvent -> GetEventID(), layerNo));
  fLayerHist -> GetYaxis() -> SetRangeUser(0, -fNumTbs);

  fLayerCvs -> Modified();
  fLayerCvs -> Update();
}

TCanvas *STPlot::GetPadplaneCanvas()
{
  PreparePadplaneHist();

  return fPadplaneCvs;
}

TCanvas *STPlot::GetSideviewCanvas()
{
  PrepareSideviewHist();

  return fSideviewCvs;
}

// Getters

// -------------------------
void STPlot::PreparePadplaneHist(EClickEvent mode)
{
//  gStyle -> SetPalette(55); // Rainbow palette - not that good

//    cvs = new TCanvas("Event Display", "", 1600, 1000); // For large monitor
  fPadplaneCvs = new TCanvas("Event Display", "", 1200, 750);
  fPadplaneCvs -> SetRightMargin(0.11);
  fPadplaneCvs -> SetLeftMargin(0.06);
  fPadplaneCvs -> SetTopMargin(0.08);
  fPadplaneCvs -> SetBottomMargin(0.08);
  fPadplaneCvs -> SetName(Form("fPadplaneCvs_%lx", (Long_t)fPadplaneCvs));
  if (mode == kDrawPad)
    fPadplaneCvs -> AddExec("DrawPad", Form("((STPlot *) STStatic::MakePointer(%ld)) -> ClickPad();", (Long_t)this));
  fPadplaneCvs -> Draw();

  fPadplaneCvs -> cd();
  fPadplaneHist = new TH2D("fPadplaneHist", ";z (mm);x (mm)", 112, 0, 1344, 108, -432, 432);
  fPadplaneHist -> SetStats(0);
  fPadplaneHist -> SetName(Form("fPadplaneHist_%lx", (Long_t)fPadplaneHist));
  fPadplaneHist -> SetTitle(Form("%s", fPadplaneTitle.Data()));
  fPadplaneHist -> GetXaxis() -> SetTitleOffset(1.0);
  fPadplaneHist -> GetXaxis() -> SetTickLength(0.01);
  fPadplaneHist -> GetXaxis() -> CenterTitle();
  fPadplaneHist -> GetYaxis() -> SetTitleOffset(0.85);
  fPadplaneHist -> GetYaxis() -> SetTickLength(0.01);
  fPadplaneHist -> GetYaxis() -> CenterTitle();
  fPadplaneHist -> Draw("colz");

  Double_t padLX = 8; // mm
  Double_t padLZ = 12; // mm

  Double_t x[2], y[2];
  for (Int_t i = 0; i < 108; i++) {
    x[0] = 0;
    x[1] = 1344;
    y[0] = -432 + (i + 1)*padLX;
    y[1] = -432 + (i + 1)*padLX;

    TGraph *graph = new TGraph(2, x, y);
    graph -> SetLineColorAlpha(kBlack, 0.3);
    graph -> SetEditable(kFALSE);

    if ((i + 1)%9 == 0)
      graph -> SetLineStyle(1);
    else
      graph -> SetLineStyle(3);

    graph -> Draw("L SAME");
  }

  for (Int_t i = 0; i < 111; i++) {
    x[0] = (i + 1)*padLZ;
    x[1] = (i + 1)*padLZ;
    y[0] = -432;
    y[1] = 432;

    TGraph *graph = new TGraph(2, x, y);
    graph -> SetLineColorAlpha(kBlack, 0.3);
    graph -> SetEditable(kFALSE);

    if ((i + 1)%7 == 0)
        graph -> SetLineStyle(1);
    else
        graph -> SetLineStyle(3);

    graph -> Draw("L SAME");
  }

  fPadplaneCvs -> Modified();
  fPadplaneCvs -> Update();
}

void STPlot::PrepareSideviewHist(EClickEvent mode)
{
  fSideviewCvs = new TCanvas("Event Display", "", 1200, 750);
  fSideviewCvs -> SetRightMargin(0.11);
  fSideviewCvs -> SetLeftMargin(0.06);
  fSideviewCvs -> SetTopMargin(0.08);
  fSideviewCvs -> SetBottomMargin(0.08);
  fSideviewCvs -> SetName(Form("fSideviewCvs_%lx", (Long_t)fSideviewCvs));
  if (mode == kDrawLayer)
    fSideviewCvs -> AddExec("DrawPad", Form("((STPlot *) STStatic::MakePointer(%ld)) -> ClickLayer();", (Long_t)this));
  fSideviewCvs -> Draw();

  fSideviewCvs -> cd();
  fSideviewHist = new TH2D("fSideviewHist", ";z (mm);y (-tb)", 112, 0, 1344, 512, -512, 0);
  fSideviewHist -> SetStats(0);
  fSideviewHist -> SetName(Form("fSideviewHist_%lx", (Long_t)fSideviewHist));
  fSideviewHist -> SetTitle(Form("%s", fSideviewTitle.Data()));
  fSideviewHist -> GetXaxis() -> SetTitleOffset(1.0);
  fSideviewHist -> GetXaxis() -> SetTickLength(0.01);
  fSideviewHist -> GetXaxis() -> CenterTitle();
  fSideviewHist -> GetYaxis() -> SetTitleOffset(0.85);
  fSideviewHist -> GetYaxis() -> SetTickLength(0.01);
  fSideviewHist -> GetYaxis() -> CenterTitle();
  fSideviewHist -> Draw("colz");

  Double_t padLX = 8; // mm
  Double_t padLZ = 12; // mm

  Double_t x[2], y[2];
  for (Int_t i = 10; i < 512; i += 10) {
    x[0] = 0;
    x[1] = 1344;
    y[0] = -i;
    y[1] = -i;

    TGraph *graph = new TGraph(2, x, y);
    graph -> SetLineColorAlpha(kBlack, 0.3);
    graph -> SetEditable(kFALSE);

    if (i%100 == 0)
      graph -> SetLineStyle(1);
    else
      graph -> SetLineStyle(3);

    graph -> Draw("L SAME");
  }

  for (Int_t i = 0; i < 111; i += 1) {
    x[0] = i*padLZ;
    x[1] = i*padLZ;
    y[0] = 0;
    y[1] = -512;

    TGraph *graph = new TGraph(2, x, y);
    graph -> SetLineColorAlpha(kBlack, 0.3);
    graph -> SetEditable(kFALSE);

    if (i%7 == 0)
      graph -> SetLineStyle(1);
    else
      graph -> SetLineStyle(3);

    graph -> Draw("L SAME");
  }

  fSideviewCvs -> Modified();
  fSideviewCvs -> Update();
}

void STPlot::PreparePadCanvas()
{
  fPadCvs = new TCanvas("fPadCvs", "", 1100, 550);
  fPadCvs -> SetRightMargin(0.03);
  fPadCvs -> SetLeftMargin(0.16);
  fPadCvs -> SetTopMargin(0.09);
  fPadCvs -> SetBottomMargin(0.11);
  fPadCvs -> SetName(Form("fPadCvs_%lx", (Long_t)fPadCvs));
  fPadCvs -> Divide(2, 1);
  fPadCvs -> Draw();
}

void STPlot::PrepareLayerHist()
{
  fLayerCvs = new TCanvas("fLayerCvs", "", 1100, 600);
  fLayerCvs -> SetRightMargin(0.11);
  fLayerCvs -> SetLeftMargin(0.06);
  fLayerCvs -> SetTopMargin(0.08);
  fLayerCvs -> SetBottomMargin(0.08);
  fLayerCvs -> SetName(Form("fLayerCvs_%lx", (Long_t)fLayerCvs));
  fLayerCvs -> Draw();

  fLayerCvs -> cd();
  fLayerHist = new TH2D("fLayerHist", "; x (-mm); y (-tb)", 108, -432, 432, 512, -512, 0);
  fLayerHist -> SetStats(0);
  fLayerHist -> SetName(Form("fLayerHist_%lx", (Long_t)fLayerHist));
  fLayerHist -> GetXaxis() -> SetTitleOffset(1.0);
  fLayerHist -> GetXaxis() -> SetTickLength(0.01);
  fLayerHist -> GetXaxis() -> CenterTitle();
  fLayerHist -> GetYaxis() -> SetTitleOffset(0.85);
  fLayerHist -> GetYaxis() -> SetTickLength(0.01);
  fLayerHist -> GetYaxis() -> CenterTitle();

  fLayerHist -> Draw("colz");

  Double_t padLX = 8; // mm
  Double_t padLZ = 12; // mm

  Double_t x[2], y[2];
  for (Int_t i = 10; i < 512; i += 10) {
    x[0] = -432;
    x[1] = 432;
    y[0] = -i;
    y[1] = -i;

    TGraph *graph = new TGraph(2, x, y);
    graph -> SetLineColorAlpha(kBlack, 0.3);
    graph -> SetEditable(kFALSE);

    if (i%100 == 0)
      graph -> SetLineStyle(1);
    else
      graph -> SetLineStyle(3);

    graph -> Draw("L SAME");
  }

  for (Int_t i = 1; i < 108; i++) {
    x[0] = (-54 + i)*padLX;
    x[1] = (-54 + i)*padLX;
    y[0] = 0;
    y[1] = -512;

    TGraph *graph = new TGraph(2, x, y);
    graph -> SetLineColorAlpha(kBlack, 0.3);
    graph -> SetEditable(kFALSE);

    if (i%9 == 0)
      graph -> SetLineStyle(1);
    else
      graph -> SetLineStyle(3);

    graph -> Draw("L SAME");
  }
}
