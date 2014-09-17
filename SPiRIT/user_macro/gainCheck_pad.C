TFile *openFile;
TCanvas *cvs;

void gainCheck_pad() {
  openFile = new TFile("GAIN_CALIBRATION_DATA.root");

  cvs = new TCanvas("gainCheck_pad", "", 1400, 600);
}

void Draw(Int_t row, Int_t layer) {
  cvs -> Clear();
  cvs -> Divide(2, 1);

  cvs -> cd(1);

  TString name = Form("pad_%d_%d", row, layer);
  TGraphErrors *pad = (TGraphErrors *) openFile -> Get(name);
  pad -> SetTitle(Form("Layer:%d Row:%d", layer, row)); 
  pad -> GetXaxis() -> SetTitle("Pulser Amplitude (V)");
  pad -> GetXaxis() -> CenterTitle();
  pad -> GetYaxis() -> SetTitle("ADC_{max. avg.} (ADC ch.)");
  pad -> GetYaxis() -> CenterTitle();
  pad -> GetYaxis() -> SetTitleOffset(1.5);
  pad -> Draw("ALP");
  TF1 *line = (TF1 *) pad -> GetFunction("pol1");
  line -> Draw("same");


  cvs -> cd(2);

  for (Int_t iVoltage = 0; iVoltage < 10; iVoltage++) {
    name = Form("hist_%d_%d_%d", row, layer, iVoltage);
    TH2D *hist = (TH2D *) openFile -> Get(name);
    hist -> GetXaxis() -> SetTitle("ADC_{max.} (ADC ch.)");
    hist -> GetXaxis() -> CenterTitle();
    hist -> GetXaxis() -> SetRangeUser(0, 4096);
    hist -> GetYaxis() -> SetTitle("Counts");
    hist -> GetYaxis() -> CenterTitle();

    if (iVoltage == 0)
      hist -> Draw();
    else
      hist -> Draw("same");

    TF1 *line = (TF1 *) hist -> GetFunction("gaus");
    line -> SetNpx(1000);
    line -> Draw("same");
  }
}
