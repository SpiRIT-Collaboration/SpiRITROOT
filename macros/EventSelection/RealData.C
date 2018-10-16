std::vector<int> LoadList(const std::string& t_filename)
{
	std::ifstream file(t_filename.c_str());
	if(!file.is_open())
		std::cerr << "Cannot open file " << t_filename << "\n";
	int index;
	std::vector<int> index_list;
	while((file >> index)) index_list.push_back(index);
	return index_list;
}

void RealData()
{
	DrawMultipleComplex mc_draw("data/Run2894_OnlyData/run2894_s*.reco.GenieTemp.1728.18d377e.root", "cbmsim");//Threshold_0.5/run2841_s0*.reco.tommy_branch.1745.1a18905.root", "cbmsim");//HighEnergy/Run_2841_full.reco.mc.root", "cbmsim");

	auto cp = mc_draw.NewCheckPoints(2);

	PID pid;
	EmbedCut gcut("PIDCut/piCuts.root", "pip");
	MomentumTracks data_mom(3);
	ValueCut data_cut(0, 200);
	ClusterNum data_cnum;
 

	pid.AddRule(cp[1]->AddRule(
                    gcut.AddRule(
                    data_mom.AddRule(
                    data_cut.AddRule(
                    data_cnum.AddRule(cp[0]))))));
                              
	TH1F data_hist("data", "", 100, 0, 100);
	TH2F PID_hist("PID", "", 300, -500, 2000, 300, 0, 800);
	auto list = LoadList("data/Run2894_OnlyData/pip_run2894.txt");
	mc_draw.DrawMultiple(list, pid, data_hist, PID_hist);

	TCanvas c1;
	data_hist.Scale(1./data_hist.GetEntries());
	data_hist.SetLineColor(kRed);
	data_hist.Draw("hist");
	DailyLogger log("Pion+_cluster_distribution");
	log.SaveClass(c1);

	new TCanvas;
	PID_hist.Draw("colz");
	gcut.GetCut()->Draw("same");
	gPad->WaitPrimitive();
}
