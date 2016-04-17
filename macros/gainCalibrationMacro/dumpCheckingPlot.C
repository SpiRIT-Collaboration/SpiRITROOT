{ // Dump all checking figures
  TFile *fGCCFile = new TFile("gainCalibration_groundPlane_########.checking.root"); // Gain calibration data checking file
  gROOT -> ProcessLine(".L DrawPadHelper.C");
  
  for (Int_t iRow = 0; iRow < 108; iRow++) {
    for (Int_t iLayer = 0; iLayer < 112; iLayer++) {
      DrawPad(iRow, iLayer);
      fPadCvs -> cd(2) -> SetLogy(0);
      fPadCvs -> cd(2) -> Update();
      fPadCvs -> SaveAs(Form("figs/pad_%d_%d.png", iRow, iLayer));
      fPadCvs -> cd(2) -> SetLogy(1);
      fPadCvs -> cd(2) -> Update();
      fPadCvs -> SaveAs(Form("figs/pad_%d_%d_log.png", iRow, iLayer));
    }
  }
}
