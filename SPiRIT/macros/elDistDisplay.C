{
  gStyle -> SetPadRightMargin(0.15);
  gStyle -> SetOptStat(11);
  gStyle -> SetStatX(0.3);             
  gStyle -> SetStatY(0.9);             

  TFile* file = new TFile("data/test.digi_youngstest.root");
         file -> cd("STDriftTask");

  TDirectory* dirDrift = (TDirectory*) file -> GetDirectory("STDriftTask");
  TDirectory* dirAvalanche = (TDirectory*) file -> GetDirectory("STAvalancheTask");

  TCanvas* cvs1 = new TCanvas("cvs1","",800,600);
  TH2D* hist1 = dirDrift -> FindObjectAny("ElDistXZ");
        hist1 -> SetNameTitle("after drift",";z (mm);x (mm)");
        hist1 -> Draw("");

  TCanvas* cvs2 = new TCanvas("cvs2","",800,600);
  TH2D* hist2 = dirDrift -> FindObjectAny("ElDistXZRaw");
        hist2 -> SetNameTitle("mc hit",";z (mm);x (mm)");
        hist2 -> Draw("");

  TCanvas* cvs3 = new TCanvas("cvs3","",800,600);
  TH2D* hist3 = dirAvalanche -> FindObjectAny("ElDistXZ");
        hist3 -> SetNameTitle("pad plane",";z (mm);x (mm)");
        hist3 -> Draw("");

  //cvs1 -> SaveAs("ElDistXZ_Drift.pdf");
  //cvs2 -> SaveAs("ElDistXZ_MC.pdf");
}
