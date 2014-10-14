{
  gStyle -> SetPadRightMargin(0.15);
  gStyle -> SetOptStat(11);
  gStyle -> SetStatX(0.3);             
  gStyle -> SetStatY(0.9);             

  TFile* file = new TFile("data/spirit.digi.root");
         file -> cd("STDriftTask");

  TDirectory* dirDrift = (TDirectory*) file -> GetDirectory("STDriftTask");
  TDirectory* dirAvalanche = (TDirectory*) file -> GetDirectory("STAvalancheTask");

  TCanvas* cvs1 = new TCanvas("cvs1","",800,600);
  TH2D* hist1 = dirDrift -> FindObjectAny("ElDistXZ");
        hist1 -> SetNameTitle("after drift",";z (cm);x (cm)");
        hist1 -> Draw("colz");

  TCanvas* cvs2 = new TCanvas("cvs2","",800,600);
  TH2D* hist2 = dirDrift -> FindObjectAny("ElDistXZRaw");
        hist2 -> SetNameTitle("mc hit",";z (cm);x (cm)");
        hist2 -> Draw("colz");

  TCanvas* cvs3 = new TCanvas("cvs3","",800,600);
  TH2D* hist3 = dirAvalanche -> FindObjectAny("ElDistXZAval");
        hist3 -> SetNameTitle("pad plane",";z (cm);x (cm)");
        hist3 -> Draw("colz");

  TCanvas* cvs4 = new TCanvas("cvs4","",700,700);
  TH2D* hist4 = dirDrift -> FindObjectAny("Dispersion");
        hist4 -> SetNameTitle("pad plane",";z (cm);x (cm)");
        hist4 -> Draw("colz");

  //cvs1 -> SaveAs("ElDistXZ_Drift.pdf");
  //cvs2 -> SaveAs("ElDistXZ_MC.pdf");
  //cvs3 -> SaveAs("ElDistXZ_Pad.pdf");
}
