#include "Bethe_formula/bethe_bloch.C"
#include "Bethe_formula/bethe_bloch_correction.C"

void plotAll()
{ 
  /** Initial Settings **/

  Bool_t fSaveFlag = kFALSE;
  Int_t  fTextSize = 0.018;
  Double_t fMaxE   = 420;

  /// 0 : proton, 1 : alpha
  Double_t massPR[2]   = {938.272, 0};
  Double_t chargePR[2] = {1, 0};

  Double_t dEdx_max = 0.005;
  Double_t dEdx_min = 0.00007;



  /** Initial Settings **/

  TString inputFileName1 = "data/energyLossProton.dat";
  TString inputFileName2 = "data/energyLossAlpha.dat";
  TString inputFileName3 = "TRIM_LISE++/data_eloss.dat";
  TString inputFileName4 = "TRIM_LISE++/data_range.dat";

  gStyle -> SetOptStat(0);
  gStyle -> SetTitleOffset(2.4,"y");
  gStyle -> SetTitleOffset(1.2,"x");
  gStyle -> SetPadLeftMargin(0.17);

  Int_t    dummy; // dummy
  Double_t p;     // particle ID : basically not needed
  Double_t m;     // momentum
  Double_t e;     // energy
  Double_t de;    // dE/dx 
  Double_t l;     // traveled length

  Int_t    index = 0; // graph counting index

  TH2D* histE = new TH2D("hist",";Energy (MeV); dE/dx (MeV/mm)",10,0,fMaxE,10,0,dEdx_max);
  // energy proton MC
  TGraph* graphEP = new TGraph();
          graphEP -> SetMarkerStyle(20);
          graphEP -> SetMarkerColor(kRed);
  // energy proton Yassid
  TGraph* graphEPy = new TGraph();
          graphEPy -> SetMarkerStyle(34);
          graphEPy -> SetMarkerColor(kBlue);
  // energy alpha MC
  TGraph* graphEA = new TGraph();
          graphEA -> SetMarkerStyle(25);
          graphEA -> SetMarkerColor(kRed);
  // energy alpha Yassid
  TGraph* graphEAy = new TGraph();
          graphEAy -> SetMarkerStyle(28);
          graphEAy -> SetMarkerColor(kBlue);

  // Bethe calculations
  TF1* betheP = new TF1("betheP",bethe_bloch,0,fMaxE,2);
       betheP -> SetLineColor(43);
       betheP -> SetLineWidth(3);
       //betheP -> SetParameters(massPR[0], chargePR[0]);
       betheP -> SetParameters(938.272, 1);
  TF1* betheCP = new TF1("betheCP",bethe_bloch_correction,0,fMaxE,2);
       //betheCP -> SetParameters(massPR[0], chargePR[0], 0);
       betheCP -> SetParameters(938.272, 1);
       betheCP -> SetLineColor(30);
       betheCP -> SetLineWidth(3);

  // Dummy
  TH1D* histELog = new TH1D("histLog",";Energy (MeV); dE/dx (MeV/mm)",10,0,fMaxE);
        histELog -> SetMinimum(dEdx_min);
        histELog -> SetMaximum(dEdx_max);

  // Traveled distance 
  TH2D* histT = new TH2D("histT",";Energy (MeV);Mean traveled distance (mm)",10,0,fMaxE,10,0,1700);
  TGraph* graphTP = new TGraph();
          graphTP -> SetMarkerStyle(20);
          graphTP -> SetMarkerColor(kRed);
  TGraph* graphTPy = new TGraph();
          graphTPy -> SetMarkerStyle(34);
          graphTPy -> SetMarkerColor(kBlue);
  TGraph* graphTA = new TGraph();
          graphTA -> SetMarkerStyle(25);
          graphTA -> SetMarkerColor(kRed);
  TGraph* graphTAy = new TGraph();
          graphTAy -> SetMarkerStyle(28);
          graphTAy -> SetMarkerColor(kBlue);

  TF1* betheTP = new TF1("betheTP",bethe_bloch_correction,0,fMaxE,3);
       betheTP -> SetLineColor(30);
       betheTP -> SetLineWidth(3);
       //betheTP -> SetParameters(massPR[0], chargePR[0], 1);
       betheCP -> SetParameters(938.272, 1, 1);







  /** Reading Data **/

  // Proton - MC  ==================================
  ifstream inputFile1;
  inputFile1.open(inputFileName1.Data());

  while(inputFile1 >> p >> m >> e >> de >> l) {
    graphEP -> SetPoint(index,e,de);
    graphTP -> SetPoint(index,e,l);
    index++;
  }

  // Alpha - MC  ===================================
  ifstream inputFile2;
  inputFile2.open(inputFileName2.Data());


  index = 0;
  while(inputFile2 >> p >> m >> e >> de >> l) {
    graphEA -> SetPoint(index,e,de);
    graphTA -> SetPoint(index,e,l);
    index++;
  }

  // TRIM/LISE++ energy ============================
  ifstream inputFile3;
  inputFile3.open(inputFileName3.Data());

  index = 0;
  while(inputFile3 >> dummy >> e >> de){
    if(dummy==0) graphEPy -> SetPoint(index,e,de);
    if(dummy==4) graphEAy -> SetPoint(index,e,de);
    index++;
  }


  // TRIM/LISE++ range =============================
  ifstream inputFile4;
  inputFile4.open(inputFileName4.Data());

  index = 0;
  while(inputFile4 >> dummy >> e >> l){

    if(dummy==0) graphTPy -> SetPoint(index,e,l*1000);
    if(dummy==4) graphTAy -> SetPoint(index,e,l*1000);
    index++;
  }





  /** Plotting Data **/

  TLegend *legendE = new TLegend(0.55,0.82,0.9,0.9);
           legendE -> AddEntry(graphEP, "proton (GEANT4)","P");
           legendE -> AddEntry(graphEPy,"proton (TRIM/LISE++)","P");
           //legendE -> AddEntry(betheP,   "proton (bethe-bloch)","L");
           //legendE -> AddEntry(betheCP,  "proton (bethe-bloch corr.)","L");
           //legendE -> AddEntry(graphEA, "alpha (GEANT4)","P");
           //legendE -> AddEntry(graphEAy,"alpha (TRIM/LISE++)","P");
           legendE -> SetTextSize(fTextSize);
           legendE -> SetFillColor(0);

  TCanvas* cvsE = new TCanvas("cvsE","",700,700);
           cvsE -> SetGrid(1,1);
  histE    -> Draw();
  legendE  -> Draw("SAME");
  graphEP  -> Draw("SAME P");
  graphEPy -> Draw("SAME P");
//  graphEA  -> Draw("SAME P");
//  graphEAy -> Draw("SAME P");
//  betheP    -> Draw("SAME");
//  betheCP   -> Draw("SAME");
  if(fSaveFlag) cvsE -> SaveAs("figures/dEdx.pdf");

  TCanvas* cvsELog = new TCanvas("cvsELog","",700,700);
           cvsELog -> SetLogy();
           cvsELog -> SetGrid(1,1);
  histELog -> Draw();
  legendE  -> Draw("SAME");
  graphEP  -> Draw("SAME P");
  //graphEA  -> Draw("SAME P");
  graphEPy -> Draw("SAME P");
  //graphEAy -> Draw("SAME P");
//  betheP    -> Draw("SAME");
//  betheCP   -> Draw("SAME");
  if(fSaveFlag) cvsELog  -> SaveAs("figures/dEdx_Log.pdf");





  //Double_t yOSL = 0.15;
  Double_t yOSL = 0;
  TLegend *legendT = new TLegend(0.58,0.80-yOSL,0.9,0.9-yOSL);
           legendT -> AddEntry(graphTP,"proton","P");
           legendT -> AddEntry(graphTPy,"proton (TRIM/LISE++)","P");
           //legendT -> AddEntry(graphTA,"alpha","P");
           //legendT -> AddEntry(graphTAy,"alpha (TRIM/LISE++)","P");
           legendT -> AddEntry(betheTP,  "proton (bethe-bloch corr.)","L");
           legendT -> SetTextSize(fTextSize);
           legendT -> SetFillColor(0);

  TCanvas* cvsT = new TCanvas("cvsT","",700,700);
           cvsT -> SetGrid(1,1);
  histT    -> Draw();
  legendT  -> Draw("SAME");
  graphTP  -> Draw("SAME P");
  //graphTA  -> Draw("SAME P");
  //graphTAy -> Draw("SAME P");
  betheTP   -> Draw("SAME");
  graphTPy -> Draw("SAME P");
  if(fSaveFlag) cvsT     -> SaveAs("figures/traveledLength.pdf");
}
