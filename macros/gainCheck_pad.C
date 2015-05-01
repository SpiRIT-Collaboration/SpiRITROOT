TFile *openFile;
TCanvas *cvs;

void gainCheck_pad() {
  openFile = new TFile("gainCalibration_pulser_20140821.root");

  cvs = new TCanvas("gainCheck_pad", "", 1400, 600);
}

void Draw(Int_t row, Int_t layer) {
  cvs -> Clear();
  cvs -> Divide(2, 1);

  cvs -> cd(1);

  TString name = Form("pad_%d_%d", row, layer);
  TGraphErrors *pad = (TGraphErrors *) openFile -> Get(name);
  pad -> Draw();
  ((TF1 *) pad -> GetFunction("pol1")) -> Draw("same");

  cvs -> cd(2);

  for (Int_t iVoltage = 0; iVoltage < 6; iVoltage++) {
    name = Form("hist_%d_%d_%d", row, layer, iVoltage);
    TH2D *hist = (TH2D *) openFile -> Get(name);
    if (iVoltage == 0)
      hist -> Draw();
    else
      hist -> Draw("same");

    ((TF1 *) hist -> GetFunction("gaus")) -> Draw("same");
  }
}
