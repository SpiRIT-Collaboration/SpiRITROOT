void plot_eloss(TString tag = "test")
{
  gStyle -> SetOptStat(0);
  gStyle -> SetTitleOffset(2.0,"y");
  gStyle -> SetTitleOffset(1.2,"x");
  gStyle -> SetPadLeftMargin(0.15);
  gStyle -> SetPadRightMargin(0.05);
  gStyle -> SetPadTopMargin(0.05);

  TH1D* hist = new TH1D("hist",";Energy (MeV);dEdx (MeV/mm)",2,0,400);
  hist -> SetMaximum(0.009);
  TGraph* graphTRIM = new TGraph();
  graphTRIM -> SetMarkerStyle(20);
  graphTRIM -> SetMarkerColor(kBlack);
  TGraph* graphG4 = new TGraph();
  graphG4 -> SetMarkerStyle(20);
  graphG4 -> SetMarkerColor(kRed);

  Double_t e, eloss, dummy;
  Int_t i;

  i=0;
  ifstream fileTRIM("data/TRIMLISE_proton_dEdx.dat");
  while(fileTRIM>>e>>eloss)
    graphTRIM -> SetPoint(i++,e,eloss);
  fileTRIM.close();

  i=0;
  ifstream fileG4("data/eloss_"+tag+".dat");
  while(fileG4>>e>>dummy>>eloss>>dummy)
    graphG4->SetPoint(i++,e*1000,eloss);
  fileG4.close();

  TCanvas* cvs = new TCanvas("cvs","",700,700);
  hist -> Draw();
  graphTRIM -> Draw("samepl");
  graphG4 -> Draw("samepl");

  TLegend *legend = new TLegend(0.4, 0.54, 0.85, 0.68);
  legend -> AddEntry(graphTRIM, "TRIM/LISE++", "pl");
  legend -> AddEntry(graphG4, "S#piRITROOT", "pl");
  legend -> SetBorderSize(0);
  legend -> Draw("same");
}
