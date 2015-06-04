void plot_eloss(TString tag = "test")
{
  Bool_t isAlpha = tag.Contains("alpha");

  gStyle -> SetPadTopMargin(0.05);      
  gStyle -> SetPadBottomMargin(0.12);   
  gStyle -> SetPadLeftMargin(0.20 - isAlpha*0.04);     
  gStyle -> SetPadRightMargin(0.04);    
  gStyle -> SetTitleOffset(1.10, "x");  
  gStyle -> SetTitleOffset(2.00 - isAlpha*0.4, "y");  
  gStyle -> SetTitleFont(132, "t");     
  gStyle -> SetTitleFont(132, "x");     
  gStyle -> SetTitleFont(132, "y");     
  gStyle -> SetLabelFont(132, "x");     
  gStyle -> SetLabelFont(132, "y");     
  gStyle -> SetTitleSize(0.05, "x");    
  gStyle -> SetTitleSize(0.05, "y");    
  gStyle -> SetLabelSize(0.05, "x");    
  gStyle -> SetLabelSize(0.05, "y");    
  gStyle -> SetLabelSize(0.05, "z");    
  gStyle -> SetLegendFillColor(0);      
  gStyle -> SetLegendFont(132);      
  gStyle -> SetEndErrorSize(4);         
  gStyle -> SetOptStat(0);

  TH1D* hist = new TH1D("hist",";Kinetic energy (MeV);dE/dx (MeV/mm)",2,0,410);
  hist -> GetXaxis() -> CenterTitle();
  hist -> GetYaxis() -> CenterTitle();
  hist -> SetMaximum((isAlpha ? 0.055 : 0.005));
  TGraph* graphTRIM = new TGraph();
  graphTRIM -> SetMarkerStyle(20);
  graphTRIM -> SetMarkerColor(kBlack);
  TGraph* graphG4 = new TGraph();
  graphG4 -> SetMarkerStyle(24);
  graphG4 -> SetMarkerColor(kRed);

  Double_t e, eloss, dummy;
  Int_t i;

  Double_t ke[44] = {0};
  Double_t eTrim[44] = {0};
  Double_t eG4[44] = {0};

  i=0;
  TString filename = Form("data/TRIMLISE_%s_dEdx.dat", (isAlpha ? "alpha" : "proton"));
  ifstream fileTRIM(filename.Data());
  while(fileTRIM>>e>>eloss) {
    ke[i] = e;
    eTrim[i] = eloss;
    graphTRIM -> SetPoint(i++,e,eloss);
  }
  fileTRIM.close();

  i=0;
  ifstream fileG4("data/eloss_"+tag+".dat");
  while(fileG4>>e>>dummy>>eloss>>dummy) {
    eG4[i] = eloss;
    graphG4->SetPoint(i++,e*1000,eloss);
  }
  fileG4.close();

  TCanvas* cvs = new TCanvas("cvs","",700,700);
  hist -> Draw();
  graphTRIM -> Draw("samepl");
  graphG4 -> Draw("samepl");

  TLegend *legend = new TLegend(0.40, 0.65, 0.95, 0.80);
  legend -> AddEntry(graphTRIM, Form("%s (TRIM/LISE++)", (isAlpha ? "alpha" : "proton")), "pl");
  legend -> AddEntry(graphG4, Form("%s (S#piRITROOT)", (isAlpha ? "alpha" : "proton")), "pl");
  legend -> SetBorderSize(0);
  legend -> Draw("same");

  for (i = 0; i < 44; i++)
    cout << setw(8) << ke[i] << " MeV, TRIM: " << setw(10) << eTrim[i] << " MeV/mm, G4: " << setw(10) << eG4[i] << " MeV/mm, \%diff: " << setw(10) << (eG4[i] - eTrim[i])/eTrim[i]*100. << endl;
}
