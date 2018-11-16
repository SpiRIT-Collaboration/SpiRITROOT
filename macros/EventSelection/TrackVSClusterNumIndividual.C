#include "EventSelection/Rules.h"

void TrackVSClusterNumIndividual()
{
	gStyle->SetPalette(kBird);
	gStyle->SetOptStat(0);

	TCanvas c1;
	c1.SetLogz();

	TCanvas c2("c2", "c2", 1500, 1200);
	c2.Divide(4,1);

        std::string filename = "Run2841_WithOffset/LowEnergy/Run_2841_mc_low_energy.reco.mc";//Run_2841_full.reco.mc";//run2841_s*.reco.develop.1737.f55eaf6";//Run_2841_full.reco.mc";//mc0_s0.reco.v1.04";
        DrawComplex drawer("data/" + filename + ".root", "cbmsim");

	DrawMultipleComplex drawer_cut("data/" + filename + ".root", "cbmsim");
	CheckPoint xz_checkpoint(0);// = drawer_cut.NewCheckPoint();
	CheckPoint yz_checkpoint(1);// = drawer_cut.NewCheckPoint();
	CheckPoint mom_checkpoint(2);// = drawer_cut.NewCheckPoint();
	CheckPoint angle_checkpoint(3);// = drawer_cut.NewCheckPoint();

	// draw cluster such that cut can be made
	std::unique_ptr<RenshengCompareData> rc_data_nocut(new RenshengCompareData);
	std::unique_ptr<RenshengCompareData> rc_data_withcut(new RenshengCompareData);
        EmbedFilter filter1, filter2;
	EmbedCut cut;
	ValueCut mom_cut(0, -1);
	DrawTrack track_xz(0, 2), track_yz(1, 2);
	MomentumTracks track_mom(3);
	ThetaPhi thetaphi;

	/*filter1.AddRule(*/rc_data_nocut.get()/*)*/;
	/*filter2.AddRule(*/rc_data_withcut->AddRule(
                        cut.AddRule(
			track_mom.AddRule(
                        mom_cut.AddRule(
                        mom_checkpoint.AddRule(
                        track_xz.AddRule(
                        xz_checkpoint.AddRule(
                        track_yz.AddRule(
                        yz_checkpoint.AddRule(
			thetaphi.AddRule(
			&angle_checkpoint))))))))))/*)*/;


	drawer_cut.SetRule(rc_data_withcut.get());//&filter2);
	

	c1.cd();
	auto hist = drawer.FillRule<TH2F>(*rc_data_nocut.get()/*filter1*/, "num_cluster_real_data", "num_cluster;Number of DB clusters;Number of clusters for MC data", 150, 0, 150, 150, 0, 150);
	hist.Draw("colz");

	TCutG* cutg = 0; 
	TCutG* temp = 0;

	// file to which all results are stored
	DailyLogger logger;
	logger.SaveClass(hist);

	int iteration = 0;

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
		logger.SaveClass(*cutg);

                TH2F hist_xz("track_distribution_xz", "Track Hit distribution;X(mm);Z(mm)", pad_x, -0.5*((double)pad_x*size_x), 0.5*((double)pad_x*size_x), pad_y, 0, (double) pad_y*size_y);
                TH2F hist_yz("track_distribution", "Track Hit distribution;Y(mm);Z(mm)", 100, -520., 0., pad_y, 0, (double) pad_y*size_y);
        	TH1F hist_mom("momentum_distribution", "Momentum Mag;Momentum (MeV/c^2)", 100, 0, 300);
		TH2F hist_angle("angle", ";Theta (deg);Phi (deg)", 90, 0, 90, 360, -180, 180);

		// Draw each tracks one by one
		for(const auto& datalist : drawer_cut)
		{
			if(datalist.size() == 0) continue;
			if(datalist[0].size() == 0) continue;
			for(const auto& row : datalist[0]) hist_xz.Fill(row[0], row[1]);
			for(const auto& row : datalist[1]) hist_yz.Fill(row[0], row[1]);
			for(const auto& row : datalist[2]) hist_mom.Fill(row[0], row[1]);
			for(const auto& row : datalist[3]) hist_angle.Fill(row[0], row[1]);

			auto pad1 = c2.cd(1);
			pad1->SetLogz();
			hist_xz.Draw("colz");

			auto pad2 = c2.cd(2);
			pad2->SetLogz();
			hist_yz.Draw("colz");

			c2.cd(3);
			hist_mom.Draw("hist");

			auto pad4 = c2.cd(4);
			pad4->SetLogz();
			hist_angle.ProjectionX()->Draw("colz");

			c2.Update();
			c2.Modified();

			// show each track for 0.1 second
			gSystem->Sleep(50);
		}

		logger.SaveClass(hist_xz);
		logger.SaveClass(hist_yz);
                logger.SaveClass(hist_mom);
		logger.SaveClass(hist_angle);
		++iteration;
		cout << "Continue? y/n";
		std::string condi;
		cin >> condi;
		if(condi == "n") break;
	}


}
