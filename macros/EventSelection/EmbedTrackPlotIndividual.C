//#include "EventSelection/Rules.h"
class RealMomObserver : public RecoTrackRule
{
public:
	virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override
	{
		auto reco_mom = track_->GetMomentum();
		std::cout << "MC momentum " << reco_mom.Mag() - t_hist.back()[0][0] << "\n";
		this->FillData(t_hist, t_entry);
	};
};

class TrackZFilter : public Rule
{
public:
	void SetMyReader(TTreeReader& t_reader) override {myTrackArray_ = std::make_shared<ReaderValue>(t_reader, "STRecoTrack");};

	virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override
	{
		bool non_empty = false;
		for(unsigned track_id_ = 0; track_id_ < (*myTrackArray_)->GetEntries(); ++track_id_)
    		{
			auto track_ = static_cast<STRecoTrack*>((*myTrackArray_) -> At(track_id_));
			for(const auto& point : (*track_->GetdEdxPointArray()))
				if(point.fPosition[2] > 10)
				{
					non_empty = true;
					break;
				}
			if(non_empty) break;
		}
		if(non_empty) this->FillData(t_hist, t_entry);
		else this->RejectData(t_hist, t_entry);
    	};

protected:
	std::shared_ptr<ReaderValue> myTrackArray_;
};


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

	DrawMultipleComplex mc_draw("data/Run2841_WithOffset/HighEnergy/Run_2841_full.reco.mc.root", "cbmsim");//HighEnergy/Run_2841_full.reco.mc.root", "cbmsim");
	DrawMultipleComplex embed_draw("data/Run2841_WithOffset/HighEnergy/run2841_s0[0-5].reco.develop.1737.f55eaf6.root", "cbmsim");

	RecoTrackNumFilter track_num_filter;
	DrawHit reco_track_xz, reco_track_yz(1,2);
	MomentumTracks reco_mom(3);
	Observer reco_obs;
	auto reco_track_xz_cp = mc_draw.NewCheckPoint();	
	auto reco_track_yz_cp = mc_draw.NewCheckPoint();

	track_num_filter.AddRule(reco_mom.AddRule(reco_obs.AddRule(reco_track_xz.AddRule(reco_track_xz_cp->AddRule(reco_track_yz.AddRule(reco_track_yz_cp))))));

	auto all_tracks_xz_cp = embed_draw.NewCheckPoint();
	auto all_tracks_yz_cp = embed_draw.NewCheckPoint();
	auto embed_mom_cp = embed_draw.NewCheckPoint();
	auto embed_tracks_xz_cp = embed_draw.NewCheckPoint();
	auto embed_tracks_yz_cp = embed_draw.NewCheckPoint();
	
	TrackZFilter min_track_num;//([](int i){return i > 3;});
	EmbedFilter filter;
	DrawTrack embed_track_xz, embed_track_yz(1,2);
        DrawTrack all_tracks_xz;
	DrawTrack all_tracks_yz(1,2);

	MomentumTracks embed_mom;
	embed_mom.SetAxis(3);
	TrackShapeFilter shape_filter("HitsHighECutG.root", 0.8);
	//Observer embed_obs;
	//RealMomObserver real_obs;
	//CompareMCPrimary comp("data/Run2841_WithOffset/LowEnergy/Momentum_distribution.txt", CompareMCPrimary::MMag, CompareMCPrimary::None);
	//ValueCut cut(-100000, 10000);
        min_track_num.AddRule(all_tracks_xz.AddRule(
                              all_tracks_xz_cp->AddRule(
                              all_tracks_yz.AddRule(
                              all_tracks_yz_cp->AddRule(
                              filter.AddRule(
	                      //embed_mom.AddRule(
                              //embed_obs.AddRule(
                              //comp.AddRule(
                              //cut.AddRule(
                              //real_obs.AddRule(
                              embed_track_xz.AddRule(
                              shape_filter.AddRule(
                              embed_tracks_xz_cp->AddRule(
                              embed_track_yz.AddRule(embed_tracks_yz_cp))))))))));

	mc_draw.SetRule(&track_num_filter);
	embed_draw.SetRule(&min_track_num);


	auto mc_it = mc_draw.begin();
	auto embed_it = embed_draw.begin();

	for(; mc_it != mc_draw.end() && embed_it != embed_draw.end(); ++mc_it, ++embed_it)
	{
		TGraph hist_mc_xz, hist_embed_xz, hist_all_xz;
		TGraph hist_mc_yz, hist_embed_yz, hist_all_yz;
		TH2F hist_xz("track_distribution_xz", "Track Hit distribution;X(mm);Z(mm)", pad_x, -0.5*((double)pad_x*size_x), 0.5*((double)pad_x*size_x), pad_y, 0, (double) pad_y*size_y);
		TLegend legend1(0.7, 0.8, 0.9, 0.9);
		TLegend legend2(0.7, 0.8, 0.9, 0.9);
		auto mc_datalist = *mc_it;
		auto embed_datalist = *embed_it;

		bool empty = false;
		for(int i = 0; i < 2; ++i)
		{
			auto& hist_mc = (i == 0)? hist_mc_xz : hist_mc_yz;
			auto& hist_embed = (i == 0)? hist_embed_xz : hist_embed_yz;
			auto& hist_all = (i == 0)? hist_all_xz : hist_all_yz;
			auto& legend = (i == 0)? legend1 : legend2;

			const auto& result_mc = mc_datalist[i];
			const auto& result_all = embed_datalist[i];
			const auto& result_embed = embed_datalist[i + 3];
			
			if(result_embed.size() == 0 || result_mc.size() == 0) 
			{
				empty = true;
				continue;
 			}
			
			for(const auto& row : result_mc) 
			{
				hist_mc.SetPoint(hist_mc.GetN(), row[0], row[1]);
				if(i == 0) hist_xz.Fill(row[0], row[1]);
			}
			bool above = false;
			for(const auto& row : result_embed) 
			{
				if(row[1] > 10) above = true;
				hist_embed.SetPoint(hist_embed.GetN(), row[0], row[1]);
			}
			for(const auto& row : result_all) hist_all.SetPoint(hist_all.GetN(), row[0], row[1]);

			c1.cd(i + 1);

			hist_mc.SetMarkerColor(kRed);
			hist_embed.SetMarkerColor(kBlue);
			hist_all.SetMarkerColor(kYellow);
			
			hist_all.SetMarkerSize(1);
			hist_all.Draw("AP");
			if(i == 0) hist_all.GetXaxis()->SetLimits(-0.5*((double)pad_x*size_x), 0.5*((double)pad_x*size_x));
			else hist_all.GetXaxis()->SetLimits(-700, 100);
			hist_all.GetYaxis()->SetRangeUser((double) pad_y*0, (double) pad_y*size_y);

			hist_mc.SetMarkerSize(0.6);
			hist_mc.Draw("P same");
			hist_embed.SetMarkerSize(0.5);
			hist_embed.Draw("P same");

			legend.AddEntry(&hist_mc, "MC hits", "p");
			legend.AddEntry(&hist_embed, "Embeded hits", "p");
			legend.Draw("same");
			c1.Modified();
			c1.Update();
		}

		// fit the xz mc hit with a curve
		if(!empty)
		{
			c1.cd(1);
			auto cutg = HistToCutG(hist_xz);
			cutg.Draw("same");
			std::cout << "Percentage inside cut " << PercentageInsideCut(cutg, hist_embed_xz) << endl;
			std::cout << "Continue ? \n";
			gPad->WaitPrimitive();
		}
	}

}
