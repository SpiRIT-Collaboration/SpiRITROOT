TString MCPEFileName = "data/MC_proton_dEdx.dat";
TString MCAEFileName = "data/MC_alpha_dEdx.dat";

TString TLPEFileName = "data/TRIMLISE++_proton_dEdx.dat";
TString TLAEFileName = "data/TRIMLISE++_alpha_dEdx.dat";

TString TLPLFileName = "data/TRIMLISE++_proton_driftLength.dat";
TString TLALFileName = "data/TRIMLISE++_alpha_driftLength.dat";

TString PEFigureName    = "figures/dEdx_proton.pdf";
TString PELogFigureName = "figures/dEdx_proton_log.pdf";

TString AEFigureName    = "figures/dEdx_alpha.pdf";
TString AELogFigureName = "figures/dEdx_alpha_log.pdf";

void plotAll()
{
  Bool_t   saveFlag  = kTRUE;
  Double_t fMaxE     = 420;
  Double_t dEdxMaxP  = 0.005;
  Double_t dEdxMinP  = 0.0002;
  Double_t dEdxMaxA  = 0.05;
  Double_t dEdxMinA  = 0.002;

  gStyle -> SetOptStat(0);
  gStyle -> SetTitleOffset(2.4,"y");
  gStyle -> SetTitleOffset(1.2,"x");
  gStyle -> SetPadLeftMargin(0.17);




  // proton dEdx
  TH2D* histPE = new TH2D("histP",";Energy (MeV); dE/dx (MeV/mm)",10,0,fMaxE,10,0,dEdxMaxP);
  TH1D* histPELog = new TH1D("histPLog",";Energy (MeV); dE/dx (MeV/mm)",10,0,fMaxE);
        histPELog -> SetMinimum(dEdxMinP);
        histPELog -> SetMaximum(dEdxMaxP);

  TGraph* graphTLPE  = GetTLPE();
          graphTLPE -> SetMarkerStyle(25);
          graphTLPE -> SetMarkerColor(kBlue);

  TGraph* graphMCPE0 = GetMCPE(0);
          graphMCPE0 -> SetMarkerStyle(20);
          graphMCPE0 -> SetMarkerColor(kGreen-3);

  TGraph* graphMCPE  = GetMCPE(1);
          graphMCPE -> SetMarkerStyle(20);
          graphMCPE -> SetMarkerColor(46);

  TLegend *legendE = new TLegend(0.43,0.80,0.9,0.9);
           legendE -> AddEntry(graphTLPE,  "proton (TRIM/LISE++)","P");
           legendE -> AddEntry(graphMCPE0, "proton (MC, secondaries not included)","P");
           legendE -> AddEntry(graphMCPE,  "proton (MC, secondaries included)","P");
           legendE -> SetFillColor(0);

  TCanvas* cvsP = new TCanvas("cvsP","cvsP",700,700);
           cvsP -> SetGrid();
  histPE      -> Draw();
  graphTLPE  -> Draw("P");
  graphMCPE0 -> Draw("P SAME");
  graphMCPE  -> Draw("P SAME");
  legendE    -> Draw("SAME");
  if(saveFlag) cvsP -> SaveAs(PEFigureName);

  TCanvas* cvsPLog = new TCanvas("cvsPLog","cvsPLog",700,700);
           cvsPLog -> SetGrid();
           cvsPLog -> SetLogy();
  histPELog   -> Draw();
  graphTLPE  -> Draw("P");
  graphMCPE0 -> Draw("P SAME");
  graphMCPE  -> Draw("P SAME");
  legendE    -> Draw("SAME");
  if(saveFlag) cvsPLog -> SaveAs(PELogFigureName);




  // alpha dEdx
  TH2D* histAE = new TH2D("histA",";Energy (MeV); dE/dx (MeV/mm)",10,0,fMaxE,10,0,dEdxMaxA);
  TH1D* histAELog = new TH1D("histALog",";Energy (MeV); dE/dx (MeV/mm)",10,0,fMaxE);
        histAELog -> SetMinimum(dEdxMinA);
        histAELog -> SetMaximum(dEdxMaxA);

  TGraph* graphTLAE  = GetTLAE();
          graphTLAE -> SetMarkerStyle(25);
          graphTLAE -> SetMarkerColor(kBlue);

  TGraph* graphMCAE0 = GetMCAE(0);
          graphMCAE0 -> SetMarkerStyle(20);
          //graphMCAE0 -> SetMarkerColor(30);
          graphMCAE0 -> SetMarkerColor(kGreen-3);

  TGraph* graphMCAE  = GetMCAE(1);
          graphMCAE -> SetMarkerStyle(20);
          graphMCAE -> SetMarkerColor(46);

  TLegend *legendE = new TLegend(0.43,0.80,0.9,0.9);
           legendE -> AddEntry(graphTLAE,  "alpha (TRIM/LISE++)","P");
           legendE -> AddEntry(graphMCAE0, "alpha (MC, secondaries not included)","P");
           legendE -> AddEntry(graphMCAE,  "alpha (MC, secondaries included)","P");
           legendE -> SetFillColor(0);

  TCanvas* cvsA = new TCanvas("cvsA","cvsA",700,700);
           cvsA -> SetGrid();
  histAE      -> Draw();
  graphTLAE  -> Draw("P");
  graphMCAE0 -> Draw("P SAME");
  graphMCAE  -> Draw("P SAME");
  legendE    -> Draw("SAME");
  if(saveFlag) cvsA -> SaveAs(AEFigureName);

  TCanvas* cvsALog = new TCanvas("cvsALog","cvsALog",700,700);
           cvsALog -> SetGrid();
           cvsALog -> SetLogy();
  histAELog   -> Draw();
  graphTLAE   -> Draw("P");
  graphMCAE0  -> Draw("P SAME");
  graphMCAE   -> Draw("P SAME");
  legendE     -> Draw("SAME");
  if(saveFlag) cvsALog -> SaveAs(AELogFigureName);




  // drift length
  //TGraph* graphMCPL= GetMCPE(2);
  //TGraph* graphTLPL= GetTLPL();
}










TGraph* GetMCPE(Int_t type = 0) // TRIM LISE++ Proton dE/dx
{
  Int_t i=0;
  Int_t dummyI;
  Double_t kE, dEdx, dEdx2, l;
  TGraph* graph = new TGraph();
  ifstream file(MCPEFileName);

  // dEdx, secondaries not included
  if(type==0) 
    while(file>>dummyI>>dummyI>>kE>>dEdx>>dEdx2>>l) 
      graph->SetPoint(i++,kE,dEdx);

  // dEdx, secondaries included
  else if(type==1) 
    while(file>>dummyI>>dummyI>>kE>>dEdx>>dEdx2>>l) 
      graph->SetPoint(i++,kE,dEdx+dEdx2);

  // drift Length
  else if(type==2)
    while(file>>dummyI>>dummyI>>kE>>dEdx>>dEdx2>>l)
      graph->SetPoint(i++,kE,l);

  return graph;
}



TGraph* GetTLPE() // TRIM LISE++ Proton dE/dx
{
  Int_t i=0;
  Double_t kE, dEdx;
  TGraph* graph = new TGraph();
  ifstream file(TLPEFileName);
  while(file>>kE>>dEdx) graph->SetPoint(i++,kE,dEdx);
  return graph;
}



TGraph* GetTLPL() // TRIM LISE++ Proton Drift Length
{
  Int_t i=0;
  Double_t kE, l;
  TGraph* graph = new TGraph();
  ifstream file(TLPLFileName);
  while(file>>kE>>l) graph->SetPoint(i++,kE,l);
  return graph;
}



TGraph* GetMCAE(Int_t type = 0) // TRIM LISE++ Proton dE/dx
{
  Int_t i=0;
  Int_t dummyI;
  Double_t kE, dEdx, dEdx2, l;
  TGraph* graph = new TGraph();
  ifstream file(MCAEFileName);

  // dEdx, secondaries not included
  if(type==0) 
    while(file>>dummyI>>dummyI>>kE>>dEdx>>dEdx2>>l) 
      graph->SetPoint(i++,kE,dEdx);

  // dEdx, secondaries included
  else if(type==1) 
    while(file>>dummyI>>dummyI>>kE>>dEdx>>dEdx2>>l) 
      graph->SetPoint(i++,kE,dEdx+dEdx2);

  // drift Length
  else if(type==2)
    while(file>>dummyI>>dummyI>>kE>>dEdx>>dEdx2>>l)
      graph->SetPoint(i++,kE,l);

  return graph;
}



TGraph* GetTLAE() // TRIM LISE++ Proton dE/dx
{
  Int_t i=0;
  Double_t kE, dEdx;
  TGraph* graph = new TGraph();
  ifstream file(TLAEFileName);
  while(file>>kE>>dEdx) graph->SetPoint(i++,kE,dEdx);
  return graph;
}



TGraph* GetTLAL() // TRIM LISE++ Proton Drift Length
{
  Int_t i=0;
  Double_t kE, l;
  TGraph* graph = new TGraph();
  ifstream file(TLALFileName);
  while(file>>kE>>l) graph->SetPoint(i++,kE,l);
  return graph;
}
