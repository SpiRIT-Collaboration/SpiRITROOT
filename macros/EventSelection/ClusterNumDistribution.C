void ClusterNumDistribution(const std::string& t_foldername = "Threshold_0.5")
{
	DrawMultipleComplex mc_draw("data/Threshold_0.5/run2841_s0*.reco.tommy_branch.1745.1a18905.root", "cbmsim");//HighEnergy/Run_2841_full.reco.mc.root", "cbmsim");

	auto cp = mc_draw.NewCheckPoints(3);

	TrackZFilter min_track_num;//([](int i){return i > 3;});
	EmbedExistence existence;
	EmbedFilter filter;
	PID pid;
	EmbedCut gcut("PIDCut/piCuts.root", "pip");
	MomentumTracks embed_mom(3);
	MomentumTracks data_mom(3);
	ValueCut embed_cut(0, 200);
	ValueCut data_cut(0, 200);
	ClusterNum embed_cnum;
	ClusterNum data_cnum;
 
        min_track_num.AddRule(filter.AddRule(
                              embed_mom.AddRule(
                              embed_cut.AddRule(
                              embed_cnum.AddRule(cp[0])))));

	filter.AddRejectRule(pid.AddRule(
                             cp[2]->AddRule(
                             gcut.AddRule(
                             data_mom.AddRule(
                             data_cut.AddRule(
                             data_cnum.AddRule(cp[1])))))));
                              
	TH1F embed_hist("embed", "", 100, 0, 100);
	TH1F data_hist("data", "", 100, 0, 100);
	TH2F PID_hist("PID", "", 300, -500, 2000, 300, 0, 800);
	mc_draw.DrawMultiple(min_track_num, embed_hist, data_hist, PID_hist);
	cout << data_hist.GetEntries() << endl;
	embed_hist.Scale(1./embed_hist.GetEntries());
	data_hist.Scale(1./data_hist.GetEntries());
	data_hist.SetLineColor(kRed);
	embed_hist.Draw("hist");
	DailyLogger logger("Embeded_pion_cluster_num");
	logger.SaveClass(embed_hist);
	data_hist.Draw("hist same");

	new TCanvas;
	PID_hist.Draw("colz");
	gcut.GetCut()->Draw("same");
	gPad->WaitPrimitive();
}
