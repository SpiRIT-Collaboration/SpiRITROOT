//////////////////////////////////////////////////////////
//                                                      //
//   Helper class                                       //
//                                                      //
//   Running this macro file alone will crash.          //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

#include "STGlobal.hh"

TCanvas *fPadCvs = NULL;
TLatex *text = NULL;
TMarker *fMarker = new TMarker();

void DrawPad(Int_t row, Int_t layer);

void DrawPadHelper() {
  TCanvas *cvs = gPad -> GetCanvas();
  TObject *select = cvs -> GetClickSelected();

  if (select == NULL || (!(select -> InheritsFrom(TH2::Class())) && !(select -> InheritsFrom(TGraph::Class()))))
    return;

  TH2D *hist = NULL;
  TIter next(cvs -> GetListOfPrimitives());
  while (TObject *obj = next()) {
    if (obj -> InheritsFrom("TH2D")) {
      hist = (TH2D *) obj;
      break;
    }
  }

  Int_t xEvent = gPad -> GetEventX();
  Int_t yEvent = gPad -> GetEventY();

  Double_t xAbs = gPad -> AbsPixeltoX(xEvent);
  Double_t yAbs = gPad -> AbsPixeltoY(yEvent);

  Double_t xOnClick = gPad -> PadtoX(xAbs);
  Double_t yOnClick = gPad -> PadtoY(yAbs);

  Int_t bin = hist -> FindBin(xOnClick, yOnClick);

  gPad -> SetUniqueID(bin);
  gPad -> GetCanvas() -> SetClickSelected(NULL);

  Int_t row = (yOnClick + 432)/8;
  Int_t layer = xOnClick/12;

  Double_t padCenterX = (row + 0.5)*8. - 432;
  Double_t padCenterZ = (layer + 0.5)*12.;

  fMarker -> SetMarkerStyle(4);
  fMarker -> SetX(padCenterZ);
  fMarker -> SetY(padCenterX);
  fMarker -> SetMarkerSize(2);
  fMarker -> Draw("same");

  for (Int_t iCvs = 0; iCvs < gROOT -> GetListOfCanvases() -> GetEntries(); iCvs++) {
    ((TCanvas *) gROOT -> GetListOfCanvases() -> At(iCvs)) -> Modified();
    ((TCanvas *) gROOT -> GetListOfCanvases() -> At(iCvs)) -> Update();
  }

  DrawPad(row, layer);
}

void DrawPad(Int_t row, Int_t layer) {
  if (fPadCvs == NULL) {
    fPadCvs = new TCanvas("padGainCheck", "", 1400, 600);
    fPadCvs -> Divide(2, 1);

    fPadCvs -> cd(1) -> SetLeftMargin(0.12);
    fPadCvs -> cd(1) -> SetRightMargin(0.05);
    fPadCvs -> cd(1) -> SetTopMargin(0.05);
    fPadCvs -> cd(1) -> SetBottomMargin(0.11);

    fPadCvs -> cd(2) -> SetLeftMargin(0.12);
    fPadCvs -> cd(2) -> SetRightMargin(0.05);
    fPadCvs -> cd(2) -> SetTopMargin(0.05);
    fPadCvs -> cd(2) -> SetBottomMargin(0.11);

    text = new TLatex();
    text -> SetNDC();
    text -> SetTextFont(132);
  }

  fPadCvs -> cd(1) -> Clear();
  fPadCvs -> cd(2) -> Clear();

  fPadCvs -> cd(1);

  TString name = Form("pad_%d_%d", row, layer);

#ifdef VVSADC
  TGraph *pad = (TGraph *) fGCCFile -> Get(name);
  pad -> GetXaxis() -> SetTitle("Amplitude (ADC)");
  pad -> GetYaxis() -> SetTitle("Pulser voltage (V)");
#else
  TGraphErrors *pad = (TGraphErrors *) fGCCFile -> Get(name);
  pad -> GetXaxis() -> SetTitle("Pulser voltage (V)");
  pad -> GetYaxis() -> SetTitle("Amplitude (ADC)");
#endif
  pad -> GetXaxis() -> SetLimits(0, 4096);
  pad -> GetXaxis() -> SetRangeUser(0, 4096);
  pad -> GetXaxis() -> CenterTitle();
  pad -> GetXaxis() -> SetTitleSize(0.05);
  pad -> GetXaxis() -> SetTitleOffset(1.00);
  pad -> GetYaxis() -> SetLimits(0, 6.);
  pad -> GetYaxis() -> SetRangeUser(0, 6.);
  pad -> GetYaxis() -> CenterTitle();
  pad -> GetYaxis() -> SetTitleSize(0.05);
  pad -> GetYaxis() -> SetTitleOffset(1.05);
  pad -> SetTitle();
  pad -> Draw("A*L");

  TF1 *fit = ((TF1 *) pad -> GetFunction("pol2"));
  fit -> Draw("same");

  fPadCvs -> cd(2);

  Int_t iVoltage = 0;
  while (kTRUE) {
    name = Form("hist_%d_%d_%d", row, layer, iVoltage);
    TH2D *hist = (TH2D *) fGCCFile -> Get(name);

    if (hist == NULL)
      break;

    hist -> SetStats(0);
    hist -> GetXaxis() -> SetTitle("Amplitude (ADC)");
    hist -> GetXaxis() -> CenterTitle();
    hist -> GetXaxis() -> SetTitleSize(0.05);
    hist -> GetXaxis() -> SetTitleOffset(1.00);
    hist -> GetXaxis() -> SetRangeUser(0, 4096);
    hist -> GetYaxis() -> SetTitle("Counts");
    hist -> GetYaxis() -> CenterTitle();
    hist -> GetYaxis() -> SetTitleSize(0.05);
    hist -> GetYaxis() -> SetTitleOffset(1.05);

    if (iVoltage == 0)
      hist -> Draw();
    else
      hist -> Draw("same");

    iVoltage++;
  }

  fPadCvs -> cd(1);
  text -> DrawLatex(0.16, 0.88, Form("(row, layer) = (%d, %d)", row, layer));
  text -> DrawLatex(0.20, 0.8, "y = A + Bx + Cx^{2}");
  text -> DrawLatex(0.25, 0.72, Form("A = %.2f#times10^{-3}", fit -> GetParameter(0)*1.E3));
  text -> DrawLatex(0.25, 0.66, Form("B = %.2f#times10^{-3}", fit -> GetParameter(1)*1.E3));
  text -> DrawLatex(0.25, 0.60, Form("C = %.2f#times10^{-9}", fit -> GetParameter(2)*1.E9));

  fPadCvs -> Modified();
  fPadCvs -> Update();
}
