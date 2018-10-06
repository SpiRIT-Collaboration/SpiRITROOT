#include "EventSelection/Rules.h"


void EmbedTrackPlotIndividual()
{
	const int pad_x = 108;
	const int pad_y = 112;
	const double size_x = 8;
	const double size_y = 12;
	const double canvas_scale = 0.5;

	gStyle->SetPalette(kBird);
	gStyle->SetMarkerStyle(8);
	TCanvas c1("canvas", "Tracks top down", 2.*canvas_scale*((double) pad_x)*size_x, canvas_scale*((double) pad_y)*size_y);
	c1.Divide(2,1);

	DrawHit reco_track_xz, reco_track_yz(1,2);
	MomentumTracks reco_mom;
	Observer reco_obs;

	reco_mom.SetAxis(3);
	reco_mom.AddRule(reco_obs.AddRule(&reco_track_xz));

	ParallelRules reco_prule;
	reco_prule.AddParallelRule(reco_mom);
	reco_prule.AddParallelRule(reco_track_yz);

	
	DrawTrackEmbed embed_track_xz, embed_track_yz(1,2);
	CheckPoint cp;
	MomentumEmbedTracks embed_mom;
	embed_mom.SetAxis(3);
	ValueCut cut(0, 10000);
	Observer obs;
	embed_mom.AddRule(cut.AddRule(obs.AddRule(embed_track_yz.AddRule(cp.AddRule(&embed_track_xz)))));

	DrawTrack all_tracks_xz;
	DrawTrack all_tracks_yz(1,2);

	ParallelRules prule;
	prule.AddParallelRule(embed_mom);
	prule.AddParallelRule(all_tracks_xz);
	prule.AddParallelRule(all_tracks_yz);
        

	FillComplex mc_draw("data/Run2841_WithOffset/HighEnergy/Run_2841_full.reco.mc.root", "cbmsim", reco_prule);
	FillComplex embed_draw("data/Run2841_WithOffset/HighEnergy/run2841_s[0-9].reco.develop.1737.f55eaf6.root", "cbmsim", prule);

	auto mc_it = mc_draw.begin();
	auto embed_it = embed_draw.begin();

	for(; mc_it != mc_draw.end() && embed_it != embed_draw.end(); ++mc_it, ++embed_it)
	{
		TGraph hist_mc_xz, hist_embed_xz, hist_all_xz;
		TGraph hist_mc_yz, hist_embed_yz, hist_all_yz;
		*mc_it;
		*embed_it;

		for(int i = 0; i < 2; ++i)
		{
			auto& hist_mc = (i == 0)? hist_mc_xz : hist_mc_yz;
			auto& hist_embed = (i == 0)? hist_embed_xz : hist_embed_yz;
			auto& hist_all = (i == 0)? hist_all_xz : hist_all_yz;

			const auto& result_mc = reco_prule.GetData(i);
			const auto& result_all = prule.GetData(i+1);
			const auto& result_embed = (i == 0)? prule.GetData(0) : cp.GetData();
			
			if(result_embed.size() == 0) continue;

			for(const auto& row : result_mc) hist_mc.SetPoint(hist_mc.GetN(), row[0], row[1]);
			for(const auto& row : result_embed) hist_embed.SetPoint(hist_embed.GetN(), row[0], row[1]);
			for(const auto& row : result_all) hist_all.SetPoint(hist_all.GetN(), row[0], row[1]);

			c1.cd(i + 1);

			hist_mc.SetMarkerColor(kRed);
			hist_embed.SetMarkerColor(kBlue);
			hist_all.SetMarkerColor(kYellow);
			
			hist_all.Draw("AP");
			if(i == 0) hist_all.GetXaxis()->SetLimits(-0.5*((double)pad_x*size_x), 0.5*((double)pad_x*size_x));
			else hist_all.GetXaxis()->SetLimits(-700, 100);
			hist_all.GetYaxis()->SetRangeUser((double) pad_y*0, (double) pad_y*size_y);

			hist_mc.Draw("P same");
			hist_embed.Draw("P same");
			c1.Modified();
			c1.Update();
		}

		std::cin.get();
	}

}
