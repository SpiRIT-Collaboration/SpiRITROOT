//plotting the correlation of the embededhits to fitted hits
void plotEmbedTrack() {
  TFile *f =  new TFile("/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/data/run2894_s0.reco.develop.1664.f68f8e0.root");//run2855_s0.reco.embed.1528.a42b22b.root");
//run2855_s0.reco.embed.1528.a42b22b.root");
  auto t = (TTree *)f->Get("cbmsim");

  TClonesArray *embedTrackArray = NULL;
  t->SetBranchAddress("STEmbedTrack",&embedTrackArray);
    
  TH1D *i_px = new TH1D("i_px","Iniital mom px",1000,0,1000);
  TH1D *f_px = new TH1D("f_px","Final mom px",1000,0,1000);
  
  TH1D *i_py = new TH1D("i_py","Iniital mom py",1000,0,1000);
  TH1D *f_py = new TH1D("f_py","Final mom py",1000,0,1000);

  TH1D *i_pz = new TH1D("i_pz","Iniital mom pz",1000,0,1000);
  TH1D *f_pz = new TH1D("f_pz","Final mom pz",1000,0,1000);

  TH1D *i_pmag = new TH1D("i_pmag","Iniital mom magnitude",1000,0,1000);
  TH1D *f_pmag = new TH1D("f_pmag","Final mom magnitude",1000,0,1000);

  for(int iEntry = 0; iEntry < t->GetEntries() ;iEntry++)
    {
      t->GetEntry(iEntry);
      for(int iTrack = 0; iTrack < embedTrackArray->GetEntries() ;iTrack++)
	{
	  auto embedTrack = (STEmbedTrack *) embedTrackArray->At(iTrack);
	TVector3 i_mom = embedTrack -> GetInitialMom();
	TVector3 f_mom = embedTrack -> GetFinalMom();

	i_px -> Fill(i_mom.Px()*1000);
	f_px -> Fill(f_mom.Px());

	i_py -> Fill(i_mom.Py()*1000);
	f_py -> Fill(f_mom.Py());

	i_pz -> Fill(i_mom.Pz()*1000);
	f_pz -> Fill(f_mom.Pz());

	i_pmag -> Fill(i_mom.Mag()*1000);
	f_pmag -> Fill(f_mom.Mag());
	}
    }

  f_px -> SetLineColor(2);
  f_py -> SetLineColor(2);
  f_pz -> SetLineColor(2);
  f_pmag -> SetLineColor(2);
  
  TCanvas *c1 = new TCanvas("c1","c1",1);
  f_px -> Draw();
  i_px -> Draw("same");
  //  c1 -> SaveAs("px.png");

  f_py -> Draw();
  i_py -> Draw("same");
  //  c1 -> SaveAs("py.png");

  f_pz -> Draw();
  i_pz -> Draw("same");
  //  c1 -> SaveAs("pz.png");

    f_pmag -> Draw();
    i_pmag -> Draw("same");

  //  c1 -> SaveAs("pmag.png");
  /*
  TFile *out = new TFile("prelimeff.root","RECREATE");
  f_px->Write();
  f_py->Write();
  f_pz->Write();
  f_pmag->Write();    

  i_px->Write();
  i_py->Write();
  i_pz->Write();
  i_pmag->Write();    

  out->Close();
  */
  
  return;
}
