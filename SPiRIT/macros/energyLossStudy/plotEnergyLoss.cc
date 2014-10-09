void plotEnergyLoss(TString inputFileName = "data/energyLossProton.dat")
{
  gStyle -> SetOptStat(0);
  gStyle -> SetTitleOffset(2.0,"y");
  gStyle -> SetTitleOffset(1.2,"x");
  gStyle -> SetPadLeftMargin(0.15);

  ifstream inputFile;
  inputFile.open(inputFileName.Data());


  TH2D* histE = new TH2D("hist","Energy loss;Energy (MeV); dE/dx (MeV/mm)",10,0,420,10,0,0.1);
  TGraph* graphEP = new TGraph();
          graphEP -> SetMarkerStyle(21);
          graphEP -> SetMarkerColor(kRed);

  TH1D* histELog = new TH1D("histLog","Energy loss (log plot);Energy (MeV); dE/dx (MeV/mm)",10,0,420);
        histELog -> SetMinimum(0.0005);
        histELog -> SetMaximum(0.1);

  TH2D* histT = new TH2D("histT","Mean traveled distance;Energy (MeV);Mean traveled distance (mm)",10,0,420,10,0,1700);
  TGraph* graphTP = new TGraph();
          graphTP -> SetMarkerStyle(21);
          graphTP -> SetMarkerColor(kRed);

  Int_t index = 0;
  Double_t p, m, e, de, l;

    cout << setw(10) << "index"
         << setw(15) << "momentum"
         << setw(15) << "energy" 
         << setw(15) << "dE/dx"
         << setw(15) << "travel length"
         << endl;
  while(inputFile >> p >> m >> e >> de >> l) {
    cout << setw(10) << index 
         << setw(15) << m 
         << setw(15) << e 
         << setw(15) << de 
         << setw(15) << l 
         << endl;
    graphEP -> SetPoint(index,e,de);
    graphTP -> SetPoint(index,e,l);
    index++;
  }



  TCanvas* cvsE = new TCanvas("cvsE","",700,700);
           cvsE -> SetGrid(1,1);
  histE -> Draw();
  graphEP -> Draw("SAME P");
  TLegend *legendE = new TLegend(0.7,0.85,0.9,0.9);
           legendE -> AddEntry(graphEP,"proton","P");
           legendE -> SetTextSize(0.035);
           legendE -> SetFillColor(0);
           legendE -> Draw("same");
  cvsE -> SaveAs("figures/dEdx.pdf");



  TCanvas* cvsELog = new TCanvas("cvsELog","",700,700);
           cvsELog -> SetLogy();
           cvsELog -> SetGrid(1,1);
  histELog -> Draw();
  graphEP  -> Draw("SAME P");
  TLegend *legendELog = new TLegend(0.7,0.85,0.9,0.9);
           legendELog -> AddEntry(graphEP,"proton","P");
           legendELog -> SetTextSize(0.035);
           legendELog -> SetFillColor(0);
           legendELog -> Draw("same");
  cvsELog -> SaveAs("figures/dEdx_Log.pdf");



  TCanvas* cvsT = new TCanvas("cvsT","",700,700);
           cvsT -> SetGrid(1,1);
  histT -> Draw();
  graphTP -> Draw("SAME P");
  TLegend *legendET = new TLegend(0.7,0.85,0.9,0.9); 
           legendET -> AddEntry(graphTP,"proton","P");
           legendET -> SetTextSize(0.035);
           legendET -> SetFillColor(0);
           legendET -> Draw("same");
  cvsT -> SaveAs("figures/traveledLength.pdf");
}
