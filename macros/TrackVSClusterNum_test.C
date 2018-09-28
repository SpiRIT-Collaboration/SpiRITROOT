#include "EventSelection/Rules.h"

void TrackVSClusterNum()
{
	gStyle->SetPalette(kBird);
	gStyle->SetOptStat(0);
	TCanvas c1;
	c1.SetLogz();

	TCanvas c2;
	c2.Divide(3,1);

	DrawMultipleComplex drawer_cut("data/Run2841_WithOffset/LowEnergy/Run_2841_mc_low_energy.reco.mc.root", "cbmsim");
	auto xz_checkpoint = drawer_cut.NewCheckPoint();
	auto yz_checkpoint = drawer_cut.NewCheckPoint();
	auto mom_checkpoint = drawer_cut.NewCheckPoint();

	// draw cluster such that cut can be made
	std::unique_ptr<RenshengCompareData> rc_data_nocut(new RenshengCompareData);
	std::unique_ptr<RenshengCompareData> rc_data_withcut(new RenshengCompareData);
	EmbedCut cut;
	DrawHit track_xz(0, 2), track_yz(1, 2);
	MomentumTracks track_mom(3);
	rc_data_withcut->AddRule(cut.AddRule(
                                 track_xz.AddRule(
                                 xz_checkpoint->AddRule(
                                 track_yz.AddRule(
                                 yz_checkpoint->AddRule(
                                 track_mom.AddRule(mom_checkpoint)))))));


	std::string filename = "Run2841_WithOffset/LowEnergy/Run_2841_mc_low_energy.reco.mc";//mc0_s0.reco.v1.04";
        DrawComplex drawer("data/" + filename + ".root", "cbmsim");


	c1.cd();
	auto hist = drawer.FillRule<TH2F>(*rc_data_nocut, "num_cluster_real_data", "num_cluster;Number of clusters for MC data;Number of DB clusters", 150, 0, 150, 150, 0, 150);
	hist.Draw("colz");

	TCutG* cutg = 0; 
	TCutG* temp = 0;

	while(true)
	{
		const int pad_x = 108;
		const int pad_y = 112;
		const double size_x = 8;
		const double size_y = 12;
		std::cout << "Waiting for cutg\n";
		while(cutg == temp)
			cutg = (TCutG*) c1.WaitPrimitive("CUTG", "CUTG");

		temp = cutg;

		// find entry number inside cut
		cut.SetCut(cutg);

                TH2F hist_xz("track_distribution_xz", "Track Hit distribution;X(mm);Z(mm)", pad_x, -0.5*((double)pad_x*size_x), 0.5*((double)pad_x*size_x), pad_y, 0, (double) pad_y*size_y);
                TH2F hist_yz("track_distribution", "Track Hit distribution;Y(mm);Z(mm)", 100, -520., 0., pad_y, 0, (double) pad_y*size_y);
        	TH1F hist_mom("momentum_distribution", "Momentum Mag;Momentum (MeV/c^2)", 100, 0, 300);
		drawer_cut.DrawMultiple(*rc_data_withcut, hist_xz, hist_yz, hist_mom);

		auto pad1 = c2.cd(1);
		pad1->SetLogz();
		hist_xz.Draw("colz");

		auto pad2 = c2.cd(2);
		pad2->SetLogz();
		hist_yz.Draw("colz");

		c2.cd(3);
		hist_mom.Draw("hist");

		c2.Update();
		c2.Modified();
	}


}
