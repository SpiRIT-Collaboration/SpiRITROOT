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

#include "Riostream.h"
#include "TStyle.h"
#include "TCanvas.h"
#include "TH2D.h"
#include "TLatex.h"
#include "TList.h"
#include "TLine.h"
#include "TString.h"

ClassImp(STMapTest);

STMapTest::STMapTest()
{
  map = new STMap();

  agetCvs = NULL;
  agetHist = NULL;

  uaCvs = NULL;
  uaHist = NULL;
  uaList = NULL;
  uaMapList = NULL;
  uaLineList = NULL;

  fIsUAMap = 0;
  fIsAGETMap = 0;
}

STMapTest::~STMapTest()
{
  if (agetHist != NULL)
    delete agetHist;
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

void STMapTest::ShowAGETMap(Int_t UAIdx)
{
  if (!fIsAGETMap || !fIsUAMap) {
    cout << "== Either AGETMap or UAMap file is not set!" << endl;

    return;
  }

  if (UAIdx%100 < 0 || UAIdx%100 > 11 || UAIdx/100 < 0 || UAIdx/100 > 3) {
    cout << "== UnitAsAd index range: ABB (A = [0, 3], BB = [00, 11])!" << endl;

    return;
  }

  if (agetHist != NULL) delete agetHist;

  gStyle -> SetOptStat(0);
  agetCvs = new TCanvas("agetCvs", "", 600, 500);
  agetHist = new TH2D("agetHist", Form("AGET Map Test (Top View) - UA%03d", UAIdx), 7, -0.5, 6.5, 9, -0.5, 8.5);
  agetHist -> GetXaxis() -> SetTitle("Local Layer Number");
  agetHist -> GetXaxis() -> CenterTitle();
  agetHist -> GetYaxis() -> SetTitle("Local Row Number");
  agetHist -> GetYaxis() -> CenterTitle();
  agetHist -> Draw("colz");

  Int_t coboIdx = map -> GetCoboIdx(UAIdx);
  Int_t asadIdx = map -> GetAsadIdx(UAIdx);

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
    textCh = NULL;
  }

  for (Int_t iLine = 0; iLine < 6; iLine++) {
    TLine *line = new TLine(iLine + 0.5, -0.5, iLine + 0.5, 8.5);
    line -> Draw("same");
  }
  for (Int_t iLine = 0; iLine < 8; iLine++) {
    TLine *line = new TLine(-0.5, iLine + 0.5, 6.5, iLine + 0.5);
    line -> Draw("same");
  }
}

void STMapTest::ShowUAMap()
{
  if (!fIsUAMap) {
    cout << "== AGETMap file is not set!" << endl;

    return;
  }

  if (uaHist == NULL) {
    gStyle -> SetOptStat(0);
    uaCvs = new TCanvas("uaCvs", "", 900, 530);
    uaHist = new TH2D("uaHist", "UnitAsAd Map Test (Top View)", 4, -0.5, 3.5, 12, -0.5, 11.5);
    uaHist -> GetXaxis() -> SetTitle("Beam Right");
    uaHist -> GetXaxis() -> CenterTitle();
    uaHist -> GetYaxis() -> SetTitle("Upstream");
    uaHist -> GetYaxis() -> CenterTitle();
    uaHist -> SetTickLength(0, "x");
    uaHist -> SetLabelColor(0, "x");
    uaHist -> SetTickLength(0, "y");
    uaHist -> SetLabelColor(0, "y");
    uaHist -> Draw("colz");

    uaMapList = new TList();
    uaList = new TList();
    for (Int_t iCobo = 0; iCobo < 12; iCobo++) {
      for (Int_t iAsad = 0; iAsad < 4; iAsad++) {
        Int_t uaIdx = map -> GetUAIdx(iCobo, iAsad);

        Int_t row = uaIdx%100;
        Int_t layer = uaIdx/100;
        TLatex *textUAMap = NULL;
        if (iCobo < 10)
          textUAMap = new TLatex(layer + 0.13, row - 0.24, Form("C%dA%d", iCobo, iAsad));
        else
          textUAMap = new TLatex(layer + 0.06, row - 0.24, Form("C%dA%d", iCobo, iAsad));

        TLatex *textUA = new TLatex(layer - 0.45, row - 0.24, Form("UA%03d", uaIdx));
        textUA -> Draw("same");
        uaList -> Add(textUA);
        textUA = NULL;

        textUAMap -> Draw("same");
        uaMapList -> Add(textUAMap);
        textUAMap = NULL;
      }
    }

    uaLineList = new TList();
    for (Int_t iLine = 0; iLine < 3; iLine++) {
      TLine *line = new TLine(iLine + 0.5, -0.5, iLine + 0.5, 11.5);
      line -> Draw("same");
      uaLineList -> Add(line);
    }
    for (Int_t iLine = 0; iLine < 11; iLine++) {
      TLine *line = new TLine(-0.5, iLine + 0.5, 3.5, iLine + 0.5);
      line -> Draw("same");
      uaLineList -> Add(line);
    }
  } else {
    gStyle -> SetOptStat(0);
    uaCvs = new TCanvas("uaCvs", "", 800, 530);
    uaHist -> Draw("colz");
 
    TIter nextUA(uaList);
    TLatex *text = NULL;
    while ((text = (TLatex *) nextUA()))
      text -> Draw("same");

    TIter nextUAMap(uaMapList);
    while ((text = (TLatex *) nextUAMap()))
      text -> Draw("same");

    TIter nextLine(uaLineList);
    TLine *line = NULL;
    while ((line = (TLine *) nextLine()))
      line -> Draw("same");
  }
}

void STMapTest::PrintMap(Int_t padRow, Int_t padLayer)
{
  Int_t uaIdx, coboIdx, asadIdx, agetIdx, chIdx;
  map -> GetMapData(padRow, padLayer, uaIdx, coboIdx, asadIdx, agetIdx, chIdx);

  cout << "============" << endl;
  cout << "   row: " << setw(3) << padRow << endl;
  cout << " layer: " << setw(3) << padLayer << endl;
  cout << endl;
  cout << "    UA: " << setw(3) << uaIdx << endl;
  cout << "  CoBo: " << setw(3) << coboIdx << endl;
  cout << "  AsAd: " << setw(3) << asadIdx << endl;
  cout << "  AGET: " << setw(3) << agetIdx << endl;
  cout << "    Ch: " << setw(3) << chIdx << endl;
  cout << "============" << endl;
}
