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

class GetEmbedInfo : public RecoTrackRule
{
public:
    GetEmbedInfo(const std::string& t_info = "")
    {
        if(t_info.empty()) return;

        std::ifstream file(t_info.c_str());
        if(!file.is_open()) std::cerr << "Cannot load info file\n";

        std::string line;
        // first line is header
        std::getline(file, line);
        while(std::getline(file, line))
        {
            std::stringstream ss(line);
            int index;
            double px, py, pz;
            if(!(ss >> index >> px >> py >> pz)) std::cerr << "Cannot read line " << line << "\n";
            p_.push_back(TVector3(1e3*px, 1e3*py, 1e3*pz));
        }
    };

    virtual void Selection(std::vector<DataSink>& t_hist, unsigned t_entry) override
    {
         current_p_ = track_->GetMomentum();
         current_i_ = t_entry;
         this->FillData(t_hist, t_entry);
    };

    std::vector<TString> TrackDescription()
    {
        auto embed_data = TString::Format("Reco info: |P| = %.1f, #theta = %.1f, #phi = %.1f", current_p_.Mag(), current_p_.Theta()*180/M_PI, current_p_.Phi()*180/M_PI);
        if(current_i_ < p_.size()) 
             return {embed_data, TString::Format("MC info: |P| = %.1f, #theta = %.1f, #phi = %.1f", p_[current_i_].Mag(), p_[current_i_].Theta()*180/M_PI, p_[current_i_].Phi()*180/M_PI)};
        return {embed_data};
    }
    
protected:
    std::vector<TVector3> p_;
    TVector3 current_p_;
    unsigned current_i_;
};


void EmbedTrackPlotNoCompare(Rule* t_Additional_rules = 0, bool t_show_rejected = false)
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

 	DrawMultipleComplex embed_draw("/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/data/Run2841_WithProton/TrackDistComp/Mom_350.0_400.0_Theta_20.0_30.0/run2841_s*","cbmsim");

	auto checkpoints = embed_draw.NewCheckPoints(5);
	TrackZFilter min_track_num;//([](int i){return i > 3;});
	EmbedFilter filter;
	DrawTrack embed_track_xz, embed_track_yz(1,2);
        DrawTrack all_tracks_xz;
	DrawTrack all_tracks_yz(1,2);
        GetEmbedInfo info("MomDist.txt");
	EmptyRule empty;

	if(!t_Additional_rules) t_Additional_rules = &empty;	
	auto block = RuleBlock(t_Additional_rules);

	MomentumTracks embed_mom;
	embed_mom.SetAxis(3);
        min_track_num.AddRule(all_tracks_xz.AddRule(
                              checkpoints[0]->AddRule(
                              all_tracks_yz.AddRule(
                              checkpoints[1]->AddRule(
                              filter.AddRule(
                              block.first))))));

	if(t_show_rejected)
		block.second->AddRejectRule(embed_track_xz.AddRule(
        	                            checkpoints[3]->AddRule(
        	                            embed_track_yz.AddRule(
                                            info.AddRule(checkpoints[4])))));
	else
		block.second->AddRule(embed_track_xz.AddRule(
        	                      checkpoints[3]->AddRule(
        	                      embed_track_yz.AddRule(
                                      info.AddRule(checkpoints[4])))));

	embed_draw.SetRule(&min_track_num);


	auto embed_it = embed_draw.begin();
	int entry = -1;


	for(; embed_it != embed_draw.end(); ++embed_it, ++embed_it)
	{
		++entry;
		TGraph hist_embed_xz, hist_all_xz;
		TGraph hist_embed_yz, hist_all_yz;
		TLegend legend1(0.5, 0.8, 0.9, 0.9);
		TLegend legend2(0.5, 0.8, 0.9, 0.9);
		auto embed_datalist = *embed_it;

		bool empty = false;
		for(int i = 0; i < 2; ++i)
		{
			auto& hist_embed = (i == 0)? hist_embed_xz : hist_embed_yz;
			auto& hist_all = (i == 0)? hist_all_xz : hist_all_yz;
			auto& legend = (i == 0)? legend1 : legend2;

			const auto& result_all = embed_datalist[i];
			const auto& result_embed = embed_datalist[i + 3];
			
			if(result_embed.size() == 0) 
			{
				empty = true;
				continue;
 			}
			
			for(const auto& row : result_embed) hist_embed.SetPoint(hist_embed.GetN(), row[0], row[1]);
			for(const auto& row : result_all) hist_all.SetPoint(hist_all.GetN(), row[0], row[1]);

			c1.cd(i + 1);

			hist_embed.SetMarkerColor(kBlue);
			hist_all.SetMarkerColor(kYellow);
			
			hist_all.SetMarkerSize(1);
			hist_all.Draw("AP");
			if(i == 0) hist_all.GetXaxis()->SetLimits(-0.5*((double)pad_x*size_x), 0.5*((double)pad_x*size_x));
			else hist_all.GetXaxis()->SetLimits(-700, 100);
			hist_all.GetYaxis()->SetRangeUser((double) pad_y*0, (double) pad_y*size_y);

			hist_embed.SetMarkerSize(0.5);
			hist_embed.Draw("P same");

			legend.AddEntry(&hist_embed, "Embeded hits", "p");
			legend.Draw("same");
			c1.Modified();
			c1.Update();
		}

		// fit the xz mc hit with a curve
		if(!empty)
		{
			c1.cd(1);
			c1.SetName(("Event_" + std::to_string(entry)).c_str());
			TPaveText description(0.1,0.9,0.99,0.99, "NDC");
                        for(auto& text : info.TrackDescription()) description.AddText(text.Data());
                        description.Draw("same");
			std::cout << "Continue ? \n";
			gPad->WaitPrimitive();
		}
	}

}
