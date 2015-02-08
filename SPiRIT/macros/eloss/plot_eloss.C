void plot_eloss(TString tag = "test")
{
  gStyle -> SetOptStat(0);
  gStyle -> SetTitleOffset(2.0,"y");
  gStyle -> SetTitleOffset(1.2,"x");
  gStyle -> SetPadLeftMargin(0.15);

  TH1D* hist = new TH1D("hist",";Energy (MeV);dEdx (MeV/mm)",2,0,100);
  hist -> SetMaximum(0.006);
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
  graphTRIM -> Draw("samep");
  graphG4 -> Draw("samep");
}
