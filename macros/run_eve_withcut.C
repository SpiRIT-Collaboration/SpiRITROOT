#include "run_eve.C"
#include "EventSelection/Rules.h"

void run_eve_withcut()
{
	gStyle->SetPalette(kBird);

	// draw cluster such that cut can be made
	std::unique_ptr<RenshengCompareData> rc_data(new RenshengCompareData);
	std::string filename = /*"Run2841_WithOffset/mc0_s0.reco.v1.04"*/"Run2841_WithOffset/LowEnergy/Run_2841_mc_low_energy.reco.mc";
        DrawComplex drawer("data/" + filename + ".root", "cbmsim");
        auto hist = drawer.FillRule<TH2F>(*rc_data, "num_cluster_real_data", "num_cluster;Number of clusters for MC data;Number of DB clusters", 150, 0, 150, 150, 0, 150);

	TCanvas c1;
	c1.SetLogz();
	hist.Draw("colz");
	auto cutg = (TCutG*) c1.WaitPrimitive("CUTG", "CUTG");

	// find entry number inside cut
	EmbedCut cut;
	cut.SetCut(cutg);
	EntryRecorder recorder;
	rc_data->AddRule(cut.AddRule(&recorder));
        drawer.FillRule<TH2F>(*rc_data, "", "", 150, 0, 150, 150, 0, 150);

	std::string evtlist("EVTLIST.dat");
	recorder.ToFile(evtlist);

	run_eve(filename.c_str(),
                "data/",
                "ST.parameters.Commissioning_201604.par",
                "",
                3000,
                0,
                false,
                "/data/Q16264/rawdataSupplement",
                TString::Format("%s", evtlist.c_str()));



}
