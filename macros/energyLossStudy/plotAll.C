TString G4PEFileName = "../../geant4/eLossProton.dat";
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
TString LFigureName     = "figures/driftLength.pdf";

Int_t Color1 = kBlue;
Int_t Color2 = kSpring-1;
Int_t Color3 = kRed-4;

Int_t MStyle1 = 24;
Int_t MStyle2 = 20;
Int_t MStyle3 = 25;
Int_t MStyle4 = 21;


void plotAll()
{
  Bool_t   saveFlag  = kTRUE;
  Double_t kEMax     = 80;
  Double_t dEdxMaxP  = 0.005;
  Double_t dEdxMinP  = 0.0002;
  Double_t dEdxMaxA  = 0.05;
  Double_t dEdxMinA  = 0.002;

  gStyle -> SetOptStat(0);
  gStyle -> SetTitleOffset(2.4,"y");
  gStyle -> SetTitleOffset(1.2,"x");
  gStyle -> SetPadLeftMargin(0.17);




  // proton dEdx
  TH2D* histPE = new TH2D("histP",";Energy (MeV); dE/dx (MeV/mm)",10,0,kEMax,10,0,dEdxMaxP);
  TH1D* histPELog = new TH1D("histPLog",";Energy (MeV); dE/dx (MeV/mm)",10,0,kEMax);
        histPELog -> SetMinimum(dEdxMinP);
        histPELog -> SetMaximum(dEdxMaxP);

  TGraph* graphTLPE  = GetTLPE();
          graphTLPE -> SetMarkerStyle(MStyle1);
          graphTLPE -> SetMarkerColor(Color1);

  TGraph* graphMCPE0 = GetMCP(0);
          graphMCPE0 -> SetMarkerStyle(MStyle2);
          graphMCPE0 -> SetMarkerColor(Color2);
          graphMCPE0 -> SetLineColor(Color2);
          graphMCPE0 -> SetLineStyle(2);

  TGraph* graphMCPE  = GetMCP(1);
          graphMCPE -> SetMarkerStyle(MStyle2);
          graphMCPE -> SetMarkerColor(Color3);
          graphMCPE -> SetLineColor(Color3);
          graphMCPE -> SetLineStyle(2);

  TGraph* graphG4PE  = GetG4P();
          graphG4PE -> SetMarkerColor(kBlack);
          graphG4PE -> SetLineColor(13);
          graphG4PE -> SetLineStyle(2);

  TLegend *legendE = new TLegend(0.55,0.80,0.9,0.9);
           legendE -> AddEntry(graphTLPE, "proton (TRIM/LISE++)","P");
           legendE -> AddEntry(graphMCPE, "proton (MC)","PL");
           legendE -> AddEntry(graphG4PE, "proton (pure Geant4)","PL");
           legendE -> SetFillColor(0);

  TCanvas* cvsP = new TCanvas("cvsP","cvsP",700,700);
           cvsP -> SetGrid();
  histPE      -> Draw();
  //graphMCPE0 -> Draw("PL");
  graphMCPE  -> Draw("PL SAME");
  graphG4PE  -> Draw("PL SAME");
  graphTLPE  -> Draw("P SAME");
  legendE    -> Draw("SAME");
  if(saveFlag) cvsP -> SaveAs(PEFigureName);

  /*
  TCanvas* cvsPLog = new TCanvas("cvsPLog","cvsPLog",700,700);
           cvsPLog -> SetGrid();
           cvsPLog -> SetLogy();
  histPELog   -> Draw();
  //graphMCPE0 -> Draw("P");
  graphMCPE  -> Draw("P SAME");
  graphG4PE  -> Draw("P SAME");
  graphTLPE  -> Draw("P SAME");
  legendE    -> Draw("SAME");
  if(saveFlag) cvsPLog -> SaveAs(PELogFigureName);
  */




  /*
  // alpha dEdx
  TH2D* histAE = new TH2D("histA",";Energy (MeV); dE/dx (MeV/mm)",10,0,kEMax,10,0,dEdxMaxA);
  TH1D* histAELog = new TH1D("histALog",";Energy (MeV); dE/dx (MeV/mm)",10,0,kEMax);
        histAELog -> SetMinimum(dEdxMinA);
        histAELog -> SetMaximum(dEdxMaxA);

  TGraph* graphTLAE  = GetTLAE();
          graphTLAE -> SetMarkerStyle(MStyle3);
          graphTLAE -> SetMarkerColor(Color1);

  TGraph* graphMCAE0 = GetMCA(0);
          graphMCAE0 -> SetMarkerStyle(MStyle4);
          //graphMCAE0 -> SetMarkerColor(30);
          graphMCAE0 -> SetMarkerColor(Color2);
          graphMCAE0 -> SetLineColor(Color2);
          graphMCAE0 -> SetLineStyle(2);

  TGraph* graphMCAE  = GetMCA(1);
          graphMCAE -> SetMarkerStyle(MStyle4);
          graphMCAE -> SetMarkerColor(Color3);
          graphMCAE -> SetLineColor(Color3);
          graphMCAE -> SetLineStyle(2);

  TLegend *legendAE = new TLegend(0.38,0.80,0.9,0.9);
           legendAE -> AddEntry(graphTLAE,  "alpha (TRIM/LISE++)","P");
           legendAE -> AddEntry(graphMCAE0, "alpha (MC, secondaries not included)","PL");
           legendAE -> AddEntry(graphMCAE,  "alpha (MC, secondaries included)","PL");
           legendAE -> SetFillColor(0);

  TCanvas* cvsA = new TCanvas("cvsA","cvsA",700,700);
           cvsA -> SetGrid();
  histAE      -> Draw();
  graphMCAE0 -> Draw("PL");
  graphMCAE  -> Draw("PL SAME");
  graphTLAE  -> Draw("P SAME");
  legendAE   -> Draw("SAME");
  if(saveFlag) cvsA -> SaveAs(AEFigureName);

  TCanvas* cvsALog = new TCanvas("cvsALog","cvsALog",700,700);
           cvsALog -> SetGrid();
           cvsALog -> SetLogy();
  histAELog  -> Draw();
  graphMCAE0 -> Draw("P");
  graphMCAE  -> Draw("P SAME");
  graphTLAE  -> Draw("P SAME");
  legendAE   -> Draw("SAME");
  if(saveFlag) cvsALog -> SaveAs(AELogFigureName);








  // drift length
  TH2D* histL = new TH2D("histL",";Energy (MeV);Mean traveled distance (mm)",10,0,70,10,0,2000);
  TGraph* graphTLPL  = GetTLPL();
          graphTLPL -> SetMarkerStyle(MStyle1);
          graphTLPL -> SetMarkerColor(Color1);

  TGraph* graphMCPL = GetMCP(2);
          graphMCPL -> SetMarkerStyle(MStyle2);
          graphMCPL -> SetMarkerColor(Color3);
          graphMCPL -> SetLineColor(Color3);
          graphMCPL -> SetLineStyle(2);

  TGraph* graphTLAL  = GetTLAL();
          graphTLAL -> SetMarkerStyle(MStyle3);
          graphTLAL -> SetMarkerColor(Color1);

  TGraph* graphMCAL = GetMCA(2);
          graphMCAL -> SetMarkerStyle(MStyle4);
          graphMCAL -> SetMarkerColor(Color3);
          graphMCAL -> SetLineColor(Color3);
          graphMCAL -> SetLineStyle(2);

  TLegend *legendL = new TLegend(0.55,0.77,0.9,0.9);
           legendL -> AddEntry(graphTLPL,  "proton (TRIM/LISE++)","P");
           legendL -> AddEntry(graphMCPL,  "proton (MC)","PL");
           legendL -> AddEntry(graphTLAL,  "alpha (TRIM/LISE++)","P");
           legendL -> AddEntry(graphMCAL,  "alpha (MC)","PL");
           legendL -> SetFillColor(0);

  TCanvas* cvsL = new TCanvas("cvsL","cvsL",700,700);
           cvsL -> SetGrid();
  histL      -> Draw();
  graphTLPL  -> Draw("P");
  graphMCPL  -> Draw("PL SAME");
  graphTLAL  -> Draw("P");
  graphMCAL  -> Draw("PL SAME");
  legendL    -> Draw("SAME");
  if(saveFlag) cvsL -> SaveAs(LFigureName);
  */
}





TGraph *GetG4P(Int_t type = 0)
{
  TGraph* graph = new TGraph();
  graph -> SetMarkerStyle(20);

  Int_t i=0;
  Double_t k, e;
  ifstream inFile(G4PEFileName);
  while(inFile >> k >> e) graph -> SetPoint(i++, k, e);

  return graph;
}





TGraph* GetMCP(Int_t type = 0) // MC Proton dE/dx
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
  while(file>>kE>>l) graph->SetPoint(i++,kE,l*1000);
  return graph;
}



TGraph* GetMCA(Int_t type = 0) // MC Alpha dE/dx
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



TGraph* GetTLAE() // TRIM LISE++ Alpha dE/dx
{
  Int_t i=0;
  Double_t kE, dEdx;
  TGraph* graph = new TGraph();
  ifstream file(TLAEFileName);
  while(file>>kE>>dEdx) graph->SetPoint(i++,kE,dEdx);
  return graph;
}



TGraph* GetTLAL() // TRIM LISE++ Alpha Drift Length
{
  Int_t i=0;
  Double_t kE, l;
  TGraph* graph = new TGraph();
  ifstream file(TLALFileName);
  while(file>>kE>>l) graph->SetPoint(i++,kE,l*1000);
  return graph;
}
