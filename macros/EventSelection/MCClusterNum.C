#include "EventSelection/Rules.h"
#include "/user/tsangc/ImproveGraph.cxx"


void MCClusterNum()
{
	gStyle->SetPalette(kBird);
	// I don't know why, but I tried not declaring it as pointer. Segmentation fault. Don't know why this works now
	RenshengCompareData *rc_data = new RenshengCompareData;
	EmbedCut cut("data/Run2841_WithOffset/CUTG.root");
	EntryRecorder recorder;

        DrawComplex drawer("data/Run2841_WithOffset/LowEnergy/Run_2841_mc_low_energy.reco.mc.root", "cbmsim");
	auto hist = drawer.FillRule<TH2F>(*rc_data, "num_cluster_real_data", "num_cluster", 150, 0, 150, 150, 0, 150);

	TCanvas c1;
	hist.Draw("colz");
	cut.GetCut()->Draw("lsame");

	rc_data->AddRule(cut.AddRule(&recorder));
	auto hist2 = drawer.FillRule<TH2F>(*rc_data, "num_cluster_real_data2", "num_cluster", 150, 0, 150, 150, 0, 150);

	auto list = recorder.GetList();
	std::ofstream evtlist("EvtList.dat");
	for(const auto& i : list)
		evtlist << i << "\n";
	evtlist.close();
	delete rc_data;

	while(true) gPad->WaitPrimitive();
}	
