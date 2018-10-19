#include "/user/tsangc/ImproveGraph.cxx"


void EmbedClusterNum()
{
	gStyle->SetPalette(kBird);
	// I don't know why, but I tried not declaring it as pointer. Segmentation fault. Don't know why this works now
	EmbedFilter filter;
	RenshengCompareData *rc_data = new RenshengCompareData;

        DrawComplex drawer("data/Run2841_WithOffset/LowEnergyIsotropic/run2841_s*.reco.tommy_branch.1746.7857ad9.root", "cbmsim");///run2841_s0[0-9].reco.develop.1737.f55eaf6.root", "cbmsim");//, "cbmsim");
	auto hist = drawer.FillRule<TH2F>(*rc_data, "num_cluster_real_data", "num_cluster", 150, 0, 150, 150, 0, 150);

	TCanvas c1;
	hist.Draw("colz");

	filter.AddRule(rc_data);
	auto hist2 = drawer.FillRule<TH2F>(filter, "num_cluster_embed", "num_cluster_embed", 150, 0, 150, 150, 0, 150);

	TCanvas c2;
        hist2.Draw("colz");
	delete rc_data;

	DailyLogger logger("RenshengCompare_Isotropic");
	logger.SaveClass(hist);
	logger.SaveClass(hist2);

	while(true) gPad->WaitPrimitive();
}	
