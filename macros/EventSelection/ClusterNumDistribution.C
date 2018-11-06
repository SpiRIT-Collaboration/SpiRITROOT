void ClusterNumDistribution(double theta_min = 0, double theta_max = 10,const std::string& t_foldername = "Threshold_0.5")
{
	DrawMultipleComplex mc_draw("data/Run2841_WithOffset/LowEnergyIsotropicNew/run2841_s*.reco.tommy_branch.1749.ef952af.root", "cbmsim");// data/Run2841_WithOffset/LowEnergyIsotropic/run2841_s*.reco.tommy_branch.1746.7857ad9.root", "cbmsim");//data/Threshold_0.5/run2841_s*.reco.tommy_branch.1745.1a18905.root", "cbmsim");//HighEnergy/Run_2841_full.reco.mc.root", "cbmsim");

	auto cp = mc_draw.NewCheckPoints(3);

	TrackZFilter min_track_num;//([](int i){return i > 3;});
	EmbedExistence existence;
	EmbedFilter filter;
	ThetaPhi thetaphi;
	ValueCut theta_cut(theta_min, theta_max);
	MomentumTracks embed_mom(3);
	ValueCut embed_50_100_cut(50, 100);
	ValueCut embed_100_150_cut(100, 150);
	ValueCut embed_150_200_cut(150, 200);
	ClusterNum embed_cnum1, embed_cnum2, embed_cnum3;
 
        min_track_num.AddRule(existence.AddRule(
                              filter.AddRule(
                              thetaphi.AddRule(
                              theta_cut.AddRule(
                              embed_mom.AddRule(
                              embed_50_100_cut.AddRule(
                              embed_cnum1.AddRule(cp[0]))))))));

	embed_50_100_cut.AddRejectRule(embed_100_150_cut.AddRule(
                                       embed_cnum2.AddRule(cp[1])));

	embed_100_150_cut.AddRejectRule(embed_150_200_cut.AddRule(
                                        embed_cnum3.AddRule(cp[2])));
	

                              
	TH1F embed_50_100_hist("embed_50_100", "", 100, 0, 100);
	TH1F embed_100_150_hist("embed_100_150", "", 100, 0, 100);
	TH1F embed_150_200_hist("embed_150_200", "", 100, 0, 100);
	mc_draw.DrawMultiple(min_track_num, embed_50_100_hist, embed_100_150_hist, embed_150_200_hist);
	new TCanvas;
	embed_50_100_hist.Draw("hist");
	embed_100_150_hist.SetLineColor(kBlue);
	embed_100_150_hist.Draw("hist same");
	embed_150_200_hist.SetLineColor(kRed);
	embed_150_200_hist.Draw("hist same");

	DailyLogger logger("Embeded_pion_cluster_num_" + std::to_string(theta_min) + "_theta_" + std::to_string(theta_max));
	logger.SaveClass(embed_50_100_hist);
	logger.SaveClass(embed_100_150_hist);
	logger.SaveClass(embed_150_200_hist);

	gPad->WaitPrimitive();
}
