//////////////////////////////////////////////////////////
//                                                      //
//   Helper class                                       //
//                                                      //
//   Running this macro file alone will crash.          //
//   Don't edit the below if you don't know about it.   //
//                                                      //
//////////////////////////////////////////////////////////

TCanvas *fPadCvs = NULL;
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
    fPadCvs = new TCanvas("padPedestalCheck", "", 1000, 700);
    fPadCvs -> Divide(2, 2);

    for (Int_t iCvs = 0; iCvs < 4; iCvs++) {
      fPadCvs -> cd(iCvs + 1) -> SetLeftMargin(0.10);
      fPadCvs -> cd(iCvs + 1) -> SetRightMargin(0.05);
      fPadCvs -> cd(iCvs + 1) -> SetTopMargin(0.09);
      fPadCvs -> cd(iCvs + 1) -> SetBottomMargin(0.10);
    }
  }

  TString fHistTitle[4] = {"Raw data", "Raw data", "FPN subtracted data", "FPN subtracted data"};
  TString fHistName[4] = {"mean_bs", "sigma_bs", "mean_as", "sigma_as"};
  TString fXTitle[4] = {"Mean ADC (ADC Ch.)", "RMS (ADC Ch.)", "Mean ADC (ADC Ch.)", "RMS (ADC Ch.)"};
  for (Int_t iCvs = 0; iCvs < 4; iCvs++) {
    fPadCvs -> cd(iCvs + 1) -> Clear();
    fPadCvs -> cd(iCvs + 1);

    TString name = Form("%s_%d_%d", fHistName[iCvs].Data(), row, layer);

    TH1D *pad = (TH1D *) fPCFile -> Get(name);
    pad -> GetXaxis() -> SetTitle(fXTitle[iCvs]);
    pad -> SetFillColor(kAzure - 2);
    pad -> SetFillStyle(3003);
    pad -> GetXaxis() -> CenterTitle();
    pad -> GetXaxis() -> SetTitleSize(0.05);
    pad -> GetXaxis() -> SetTitleOffset(1.00);
    pad -> GetYaxis() -> SetTitle("Counts");
    pad -> GetYaxis() -> CenterTitle();
    pad -> GetYaxis() -> SetTitleSize(0.05);
    pad -> GetYaxis() -> SetTitleOffset(1.05);
    if (iCvs%2 == 0) {
      pad -> GetXaxis() -> SetRangeUser(pad -> GetBinCenter(pad -> FindFirstBinAbove()) - 10, pad -> GetBinCenter(pad -> FindLastBinAbove()) + 10);
    } else {
      pad -> GetXaxis() -> SetRangeUser(0, pad -> GetBinCenter(pad -> FindLastBinAbove()) + 5);
    }
    pad -> SetTitle(fHistTitle[iCvs]);
    pad -> Draw();

    fPadCvs -> cd(iCvs + 1) -> Modified();
    fPadCvs -> cd(iCvs + 1) -> Update();
  }
}
