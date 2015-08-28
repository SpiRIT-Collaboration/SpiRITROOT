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

#include "TCanvas.h"
#include "TStyle.h"
#include "TH1D.h"
#include "TH2D.h"
#include "TLine.h"

#include <iostream>

ClassImp(STPlot)

STPlot::STPlot()
{
  Clear();
}

STPlot::STPlot(STCore *core)
{
  Clear();

  fCore = core;
  SetNumTbs(fCore -> GetNumTbs());
}

void STPlot::Clear()
{
  fEvent = NULL;
  fNumTbs = 512;

  padplaneCvs = NULL;
  padplaneHist = NULL;

  padCvs = NULL;
  padGraph[0] = NULL;
  padGraph[1] = NULL;
}

Bool_t STPlot::CheckEvent()
{
  if (fEvent == NULL) {
    std::cerr << "= [STPlot] Event is not set!" << std::endl;
    return 1;
  }

  return 0;
}

// Setters
void STPlot::SetEvent(STRawEvent *anEvent)
{
  fEvent = anEvent;

  std::cerr << "= [STPlot] Default number of time buckets is 512." << std::endl;
}

void STPlot::SetNumTbs(Int_t numTbs)
{
  fNumTbs = numTbs;
}

void STPlot::DrawPadplane()
{
  if (CheckEvent())
    return;

  if (padplaneHist)
    padplaneHist -> Reset();
  else
    PreparePadplaneHist();

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

    padplaneHist -> SetBinContent(aPad -> GetLayer() + 1, aPad -> GetRow() + 1, maxADC);
    if (maxADC > max) max = maxADC;
  }

  padplaneHist -> SetMaximum(max);
}

void STPlot::DrawPad(Int_t row, Int_t layer)
{
  if (padCvs == NULL)
    PreparePadCanvas();
  else {
    padGraph[0] -> Set(0);
    padGraph[1] -> Set(0);
  }

  if (CheckEvent())
    return;

  STPad *pad = fEvent -> GetPad(row, layer);
  if (!pad) {
    std::cerr << "= [STPlot] There's no pad (" << row << ", " << layer << ")!" << std::endl;
    return;
  }

  Int_t *tempRawAdc = pad -> GetRawADC();
  Double_t tb[512] = {0};
  Double_t rawAdc[512] = {0};
  for (Int_t iTb = 0; iTb < fNumTbs; iTb++) {
    tb[iTb] = iTb;
    rawAdc[iTb] = tempRawAdc[iTb];
  }

  padGraph[0] = new TGraph(fNumTbs, tb, rawAdc);
  padGraph[0] -> SetTitle(Form("Raw ADC - (%d, %d)", row, layer));
  padGraph[0] -> SetLineColor(2);
  padGraph[0] -> GetHistogram() -> GetXaxis() -> SetTitle("Time bucket");
  padGraph[0] -> GetHistogram() -> GetXaxis() -> CenterTitle();
  padGraph[0] -> GetHistogram() -> GetYaxis() -> SetTitle("ADC");
  padGraph[0] -> GetHistogram() -> GetYaxis() -> CenterTitle();
  padGraph[0] -> GetHistogram() -> GetYaxis() -> SetLimits(-10, 4106);
  padGraph[0] -> GetHistogram() -> GetYaxis() -> SetRangeUser(-10, 4106);

  padCvs -> cd(1);
  padGraph[0] -> Draw("AL");

  Double_t *adc = pad -> GetADC();
  padGraph[1] = new TGraph(fNumTbs, tb, adc);
  padGraph[1] -> SetTitle(Form("ADC(FPN pedestal subtracted) - (%d, %d)", row, layer));
  padGraph[1] -> SetLineColor(2);
  padGraph[1] -> GetHistogram() -> GetXaxis() -> SetTitle("Time bucket");
  padGraph[1] -> GetHistogram() -> GetXaxis() -> CenterTitle();
  padGraph[1] -> GetHistogram() -> GetYaxis() -> SetTitle("ADC");
  padGraph[1] -> GetHistogram() -> GetYaxis() -> CenterTitle();
  padGraph[1] -> GetHistogram() -> GetYaxis() -> SetLimits(-10, 4106);
  padGraph[1] -> GetHistogram() -> GetYaxis() -> SetRangeUser(-10, 4106);

  padCvs -> cd(2);
  padGraph[1] -> Draw("AL");
}

void STPlot::DrawLayer(Int_t layerNo)
{
  std::cerr << "= [STPlot] Not Implemented!" << std::endl;

  if (layerHist != NULL)
    delete layerHist;

  if (CheckEvent())
    return;
}
// Getters

// -------------------------
void STPlot::PreparePadplaneHist()
{
//    cvs = new TCanvas("Event Display", "", 1600, 1000); // For large monitor
    padplaneCvs = new TCanvas("Event Display", "", 1200, 750);
    padplaneCvs -> Draw();

    gStyle -> SetOptStat(0000);
    gStyle -> SetPadRightMargin(0.08);
    gStyle -> SetPadLeftMargin(0.06);
    gStyle -> SetPadTopMargin(0.04);
    gStyle -> SetPadBottomMargin(0.08);
    gStyle -> SetTitleOffset(1.0, "X");
    gStyle -> SetTitleOffset(0.85, "Y");

    padplaneCvs -> cd();
    padplaneHist = new TH2D("padplaneHist", ";x (mm);z (mm)", 112, 0, 1344, 108, -432, 432);
    padplaneHist -> GetXaxis() -> SetTickLength(0.01);
    padplaneHist -> GetXaxis() -> CenterTitle();
    padplaneHist -> GetYaxis() -> SetTickLength(0.01);
    padplaneHist -> GetYaxis() -> CenterTitle();
    padplaneHist -> Draw("colz");

    Double_t padLX = 8; // mm
    Double_t padLZ = 12; // mm

    Double_t x[2], y[2];
    for (Int_t i = 0; i < 107; i++) {
      x[0] = 0;
      x[1] = 1344;
      y[0] = -432 + (i + 1)*padLX;
      y[1] = -432 + (i + 1)*padLX;

      TGraph *graph = new TGraph(2, x, y);
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
      y[1] = -432;

      TGraph *graph = new TGraph(2, x, y);
      graph -> SetEditable(kFALSE);

      if ((i + 1)%7 == 0)
          graph -> SetLineStyle(1);
      else
          graph -> SetLineStyle(3);

      graph -> Draw("L SAME");
    }
}

void STPlot::PreparePadCanvas()
{
  gStyle -> SetOptStat(0000);
  gStyle -> SetPadRightMargin(0.03);
  gStyle -> SetPadLeftMargin(0.16);
  gStyle -> SetPadTopMargin(0.09);
  gStyle -> SetPadBottomMargin(0.11);
  gStyle -> SetTitleOffset(1.05, "X");
  gStyle -> SetTitleOffset(1.75, "Y");
  gStyle -> SetTitleSize(0.05, "X");
  gStyle -> SetTitleSize(0.05, "Y");
  gStyle -> SetLabelSize(0.05, "X");
  gStyle -> SetLabelSize(0.05, "Y");

  padCvs = new TCanvas("padCvs", "", 1100, 550);
  padCvs -> Divide(2, 1);
  padCvs -> Draw();
}
