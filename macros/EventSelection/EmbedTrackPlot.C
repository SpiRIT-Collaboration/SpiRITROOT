#include "EventSelection/Rules.h"

void EmbedTrackPlot()
{
	const int pad_x = 108;
	const int pad_y = 112;
	const double size_x = 8;
	const double size_y = 12;

	gStyle->SetPalette(kBird);
	TCanvas c1;
	c1.SetLogz();

	// draw cluster such that cut can be made

	std::string filename = /*"run2841_s[0-5].reco.develop.1737.f55eaf6";*/"Run2841_WithOffset/Run_2841_full.reco.mc";//mc0_s0.reco.v1.04";
        DrawComplex drawer("data/" + filename + ".root", "cbmsim");

	DrawTrack track;
        TH2F hist("track_distribution", "Track Hit distribution;X(mm);Y(mm)", pad_x, -0.5*((double)pad_x*size_x), 0.5*((double)pad_x*size_x), pad_y, 0, (double) pad_y*size_y);

	c1.cd();
	drawer.FillRule(track, hist);
	hist.Draw("colz");

	c1.WaitPrimitive("CUTG");


}
