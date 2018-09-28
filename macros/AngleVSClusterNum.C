#include "EventSelection/Rules.h"

void AngleVSClusterNum()
{
	gStyle->SetPalette(kBird);
	TCanvas c1;
	c1.SetLogz();
	TCanvas c2;
	c2.SetLogz();
	//c2.Divide(2,1);

	// draw cluster such that cut can be made
	std::unique_ptr<RenshengCompareData> rc_data_nocut(new RenshengCompareData);
	std::unique_ptr<RenshengCompareData> rc_data_withcut(new RenshengCompareData);
	EmbedCut cut;
	ThetaPhi thetaphi;
	rc_data_withcut->AddRule(cut.AddRule(&thetaphi));

	std::string filename = "Run2841_WithOffset/Run_2841_full.reco.mc";//mc0_s0.reco.v1.04";
        DrawComplex drawer("data/" + filename + ".root", "cbmsim");


        TH2F hist("num_cluster_real_data", "num_cluster;Number of clusters for MC data;Number of DB clusters", 150, 0, 150, 150, 0, 150);

	c1.cd();
	drawer.FillRule(*rc_data_nocut, hist);
	hist.Draw("colz");

	while(true)
	{
		auto cutg = (TCutG*) c1.WaitPrimitive("CUTG", "CUTG");

		// find entry number inside cut
		TH2F hist2("angle", ";Theta (deg);Phi (deg)", 90, 0, 90, 360, -180, 180);
		cut.SetCut(cutg);
        	drawer.FillRule(*rc_data_withcut, hist2);

		c2.cd();
		hist2.Draw("colz");
		//c2.cd(2);
		//hist2.ProjectionX()->Draw("hist");
		c2.Update();
		c2.Modified();
	}


}
