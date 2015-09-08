// =================================================
//  STMapTest Class
// 
//  Description:
//    Test AGET & UnitAsAd map with plot
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2013. 08. 30
// =================================================

#include "STMapTest.hh"

#include "TStyle.h"
#include "TCanvas.h"
#include "TGraph.h"
#include "TH2D.h"
#include "TLatex.h"
#include "TLine.h"
#include "TString.h"

#include <iostream>
#include <iomanip>

ClassImp(STMapTest);

STMapTest::STMapTest()
{
  map = new STMap();

  agetCvs[0] = NULL;
  agetCvs[1] = NULL;
  agetHist[0] = NULL;
  agetHist[1] = NULL;

  uaCvs = NULL;
  uaHist = NULL;

  fIsUAMap = 0;
  fIsAGETMap = 0;
}

STMapTest::~STMapTest()
{
}

void STMapTest::SetUAMap(TString filename)
{
  map -> SetUAMap(filename);

  fIsUAMap = map -> IsSetUAMap();
}

void STMapTest::SetAGETMap(TString filename)
{
  map -> SetAGETMap(filename);

  fIsAGETMap = map -> IsSetAGETMap();
}

void STMapTest::ShowAGETMap()
{
  if (!fIsAGETMap || !fIsUAMap) {
    std::cout << "== Either AGETMap or UAMap file is not set!" << std::endl;

    return;
  }

  gStyle -> SetOptStat(0);
  gStyle -> SetTitleSize(0.05, "x");
  gStyle -> SetTitleSize(0.05, "y");
  gStyle -> SetTitleOffset(0.90, "x");
  gStyle -> SetTitleOffset(0.85, "y");
  agetCvs[0] = new TCanvas("agetCvsLeft", "", 900, 0, 600, 500);
  agetHist[0] = new TH2D("agetHistLeft", "AGET Mapping (Beam left half)", 7, -0.5, 6.5, 9, -0.5, 8.5);
  agetHist[0] -> GetXaxis() -> SetTitle("Layer Number in AGET (Upstream)");
  agetHist[0] -> GetXaxis() -> CenterTitle();
  agetHist[0] -> GetYaxis() -> SetTitle("Row Number in AGET (Beam right)");
  agetHist[0] -> GetYaxis() -> CenterTitle();
  agetHist[0] -> Draw("colz");

  Int_t coboIdx = map -> GetCoboIdx(0);
  Int_t asadIdx = map -> GetAsadIdx(0);

  for (Int_t iCh = 0; iCh < 68; iCh++) {
    Int_t row, layer;
    map -> GetRowNLayer(coboIdx, asadIdx, 0, iCh, row, layer);

    if (row == -2 || layer == -2)
      continue;

    TLatex *textCh = NULL;
    if (iCh < 10)
      textCh = new TLatex(layer%28%7 - 0.07, row%9 - 0.16, Form("%d", iCh));
    else 
      textCh = new TLatex(layer%28%7 - 0.19, row%9 - 0.16, Form("%d", iCh));

    textCh -> Draw("same");
  }

  for (Int_t iLine = 0; iLine < 6; iLine++) {
    TGraph *line = new TGraph();
    line -> SetPoint(0, iLine + 0.5, -0.5);
    line -> SetPoint(1, iLine + 0.5, 8.5);
    line -> Draw("same");
  }

  for (Int_t iLine = 0; iLine < 8; iLine++) {
    TGraph *line = new TGraph();
    line -> SetPoint(0, -0.5, iLine + 0.5);
    line -> SetPoint(1, 6.5, iLine + 0.5);
    line -> Draw("same");
  }

  agetCvs[0] -> SetEditable(kFALSE);

  agetCvs[1] = new TCanvas("agetCvsRight", "", 900, 545, 600, 500);
  agetHist[1] = new TH2D("agetHistRight", "AGET Mapping (Beam right half)", 7, -0.5, 6.5, 9, -0.5, 8.5);
  agetHist[1] -> GetXaxis() -> SetTitle("Layer Number in AGET (Upstream)");
  agetHist[1] -> GetXaxis() -> CenterTitle();
  agetHist[1] -> GetYaxis() -> SetTitle("Row Number in AGET (Beam right)");
  agetHist[1] -> GetYaxis() -> CenterTitle();
  agetHist[1] -> Draw("colz");

  coboIdx = map -> GetCoboIdx(6);
  asadIdx = map -> GetAsadIdx(6);

  for (Int_t iCh = 0; iCh < 68; iCh++) {
    Int_t row, layer;
    map -> GetRowNLayer(coboIdx, asadIdx, 0, iCh, row, layer);

    if (row == -2 || layer == -2)
      continue;

    TLatex *textCh = NULL;
    if (iCh < 10)
      textCh = new TLatex(layer%28%7 - 0.07, row%9 - 0.16, Form("%d", iCh));
    else 
      textCh = new TLatex(layer%28%7 - 0.19, row%9 - 0.16, Form("%d", iCh));

    textCh -> Draw("same");
  }

  for (Int_t iLine = 0; iLine < 6; iLine++) {
    TGraph *line = new TGraph();
    line -> SetPoint(0, iLine + 0.5, -0.5);
    line -> SetPoint(1, iLine + 0.5, 8.5);
    line -> Draw("same");
  }

  for (Int_t iLine = 0; iLine < 8; iLine++) {
    TGraph *line = new TGraph();
    line -> SetPoint(0, -0.5, iLine + 0.5);
    line -> SetPoint(1, 6.5, iLine + 0.5);
    line -> Draw("same");
  }

  agetCvs[1] -> SetEditable(kFALSE);
}

void STMapTest::ShowUAMap()
{
  if (!fIsUAMap) {
    std::cout << "== UAMap file is not set!" << std::endl;

    return;
  }

  gStyle -> SetOptStat(0);
  gStyle -> SetTitleSize(0.05, "x");
  gStyle -> SetTitleSize(0.05, "y");
  gStyle -> SetTitleOffset(0.55, "x");
  gStyle -> SetTitleOffset(0.4, "y");
  gStyle -> SetPadLeftMargin(0.05);
  gStyle -> SetPadRightMargin(0.05);
  gStyle -> SetPadTopMargin(0.08);
  gStyle -> SetPadBottomMargin(0.08);
  gStyle -> SetLabelOffset(9999., "x");
  gStyle -> SetLabelOffset(9999., "y");

  uaCvs = new TCanvas("uaCvs", "", 900, 530);
  uaHist = new TH2D("uaHist", "AsAd Mapping (Top View)", 4, -0.5, 3.5, 12, -0.5, 11.5);
  uaHist -> GetXaxis() -> SetTitle("Beam Right");
  uaHist -> GetXaxis() -> CenterTitle();
  uaHist -> GetYaxis() -> SetTitle("Upstream");
  uaHist -> GetYaxis() -> CenterTitle();
  uaHist -> SetTickLength(0, "x");
  uaHist -> SetLabelColor(0, "x");
  uaHist -> SetTickLength(0, "y");
  uaHist -> SetLabelColor(0, "y");
  uaHist -> Draw("colz");

  for (Int_t iCobo = 0; iCobo < 12; iCobo++) {
    for (Int_t iAsad = 0; iAsad < 4; iAsad++) {
      Int_t uaIdx = map -> GetUAIdx(iCobo, iAsad);

      if (uaIdx == -1)
        continue;

      Int_t row = uaIdx%100;
      Int_t layer = uaIdx/100;
      TLatex *textUAMap = NULL;
      if (iCobo < 10)
        textUAMap = new TLatex(layer + 0.13, row - 0.24, Form("C%dA%d", iCobo, iAsad));
      else
        textUAMap = new TLatex(layer + 0.06, row - 0.24, Form("C%dA%d", iCobo, iAsad));

      TLatex *textUA = new TLatex(layer - 0.45, row - 0.24, Form("UA%03d", uaIdx));
      textUA -> Draw("same");

      textUAMap -> Draw("same");
    }
  }

  for (Int_t iLine = 0; iLine < 3; iLine++) {
    TGraph *line = new TGraph();
    line -> SetPoint(0, iLine + 0.5, -0.5);
    line -> SetPoint(1, iLine + 0.5, 11.5);
    line -> Draw("same");
  }

  for (Int_t iLine = 0; iLine < 11; iLine++) {
    TGraph *line = new TGraph();
    line -> SetPoint(0, -0.5, iLine + 0.5);
    line -> SetPoint(1,  3.5, iLine + 0.5);
    if (iLine == 5)
      line -> SetLineWidth(3);
    line -> Draw("same");
  }

  uaCvs -> SetEditable(kFALSE);
}

void STMapTest::PrintMap(Int_t padRow, Int_t padLayer)
{
  Int_t uaIdx, coboIdx, asadIdx, agetIdx, chIdx;
  map -> GetMapData(padRow, padLayer, uaIdx, coboIdx, asadIdx, agetIdx, chIdx);

  std::cout << "============" << std::endl;
  std::cout << "   row: " << std::setw(3) << padRow << std::endl;
  std::cout << " layer: " << std::setw(3) << padLayer << std::endl;
  std::cout << std::endl;
  std::cout << "    UA: " << std::setw(3) << Form("%03d", uaIdx) << std::endl;
  std::cout << "  CoBo: " << std::setw(3) << coboIdx << std::endl;
  std::cout << "  AsAd: " << std::setw(3) << asadIdx << std::endl;
  std::cout << "  AGET: " << std::setw(3) << agetIdx << std::endl;
  std::cout << "    Ch: " << std::setw(3) << chIdx << std::endl;
  std::cout << "============" << std::endl;
}

void STMapTest::PrintMap(Int_t coboIdx, Int_t asadIdx, Int_t agetIdx, Int_t chIdx)
{
  Int_t padRow, padLayer;
  Int_t uaIdx = map -> GetUAIdx(coboIdx, asadIdx);
  map -> GetRowNLayer(coboIdx, asadIdx, agetIdx, chIdx, padRow, padLayer);

  std::cout << "============" << std::endl;
  std::cout << "   row: " << std::setw(3) << padRow << std::endl;
  std::cout << " layer: " << std::setw(3) << padLayer << std::endl;
  std::cout << std::endl;
  std::cout << "    UA: " << std::setw(3) << Form("%03d", uaIdx) << std::endl;
  std::cout << "  CoBo: " << std::setw(3) << coboIdx << std::endl;
  std::cout << "  AsAd: " << std::setw(3) << asadIdx << std::endl;
  std::cout << "  AGET: " << std::setw(3) << agetIdx << std::endl;
  std::cout << "    Ch: " << std::setw(3) << chIdx << std::endl;
  std::cout << "============" << std::endl;
}
