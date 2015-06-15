void plot_all()
{
  gStyle -> SetPadTopMargin(0.05);      
  gStyle -> SetPadBottomMargin(0.13);   
  gStyle -> SetPadLeftMargin(0.17);     
  gStyle -> SetPadRightMargin(0.04);    
  gStyle -> SetTitleSize(0.06, "x");    
  gStyle -> SetTitleSize(0.06, "y");    
  gStyle -> SetTitleOffset(1.10, "x");  
  gStyle -> SetTitleOffset(1.35, "y");  
  gStyle -> SetTitleFont(132, "t");     
  gStyle -> SetTitleFont(132, "x");     
  gStyle -> SetTitleFont(132, "y");     
  gStyle -> SetLabelFont(132, "x");     
  gStyle -> SetLabelFont(132, "y");     
  gStyle -> SetTitleSize(0.06, "x");    
  gStyle -> SetTitleSize(0.06, "y");    
  gStyle -> SetLabelSize(0.06, "x");    
  gStyle -> SetLabelSize(0.06, "y");    
  gStyle -> SetLabelSize(0.06, "z");    
  gStyle -> SetOptStat(0);
  gStyle -> SetLegendFillColor(0);      
  gStyle -> SetLegendFont(132);      
  gStyle -> SetEndErrorSize(4);         

  TH1D* hist = new TH1D("hist",";Energy (MeV);dEdx (MeV/mm)",450,0,410);
  hist -> SetMaximum(0.055);
  hist -> GetXaxis() -> CenterTitle();
  hist -> GetYaxis() -> CenterTitle();
  TGraph* graphTRIM_proton = new TGraph();
  graphTRIM_proton -> SetMarkerStyle(20);
  graphTRIM_proton -> SetMarkerColor(kBlack);
  TGraph* graphG4_proton = new TGraph();
  graphG4_proton -> SetMarkerStyle(24);
  graphG4_proton -> SetMarkerColor(kRed);
  TGraph* graphTRIM_alpha = new TGraph();
  graphTRIM_alpha -> SetMarkerStyle(21);
  graphTRIM_alpha -> SetMarkerColor(kBlack);
  TGraph* graphG4_alpha = new TGraph();
  graphG4_alpha -> SetMarkerStyle(25);
  graphG4_alpha -> SetMarkerColor(kRed);

  Double_t e, eloss, dummy;
  Int_t i;

  i=0;
  TString filename = "data/TRIMLISE_proton_dEdx.dat";
  ifstream fileTRIM(filename.Data());
  while(fileTRIM>>e>>eloss)
    graphTRIM_proton -> SetPoint(i++,e,eloss);
  fileTRIM.close();

  i=0;
  ifstream fileG4("data/eloss_VMCG4_proton.dat");
  while(fileG4>>e>>dummy>>eloss>>dummy)
    graphG4_proton->SetPoint(i++,e*1000,eloss);
  fileG4.close();

  i=0;
  TString filename = "data/TRIMLISE_alpha_dEdx.dat";
  ifstream fileTRIM(filename.Data());
  while(fileTRIM>>e>>eloss)
    graphTRIM_alpha -> SetPoint(i++,e,eloss);
  fileTRIM.close();

  i=0;
  ifstream fileG4("data/eloss_VMCG4_alpha.dat");
  while(fileG4>>e>>dummy>>eloss>>dummy)
    graphG4_alpha->SetPoint(i++,e*1000,eloss);
  fileG4.close();

  TCanvas* cvs = new TCanvas("cvs","",700,700);
  hist -> Draw();
  graphTRIM_proton -> Draw("samepl");
  graphG4_proton -> Draw("samepl");
  graphTRIM_alpha -> Draw("samepl");
  graphG4_alpha -> Draw("samepl");

  TLegend *legend = new TLegend(0.35, 0.60, 0.95, 0.88);
  legend -> AddEntry(graphTRIM_alpha, "alpha (TRIM/LISE++)", "pl");
  legend -> AddEntry(graphG4_alpha, "alpha (S#piRITROOT)", "pl");
  legend -> AddEntry(graphTRIM_proton, "proton (TRIM/LISE++)", "pl");
  legend -> AddEntry(graphG4_proton, "proton (S#piRITROOT)", "pl");
  legend -> SetBorderSize(0);
  legend -> Draw("same");
}
