//#include "EventSelection/Rules.h"
class ShowEntry : public Rule
{
public:
	virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override
	{
		std::cout << "Entry " << t_entry << "\n";
		this->FillData(t_hist, t_entry);
	};
    
	std::unique_ptr<Rule> Clone() override
	{
		return std::make_unique<ShowEntry>(*this);
	};
};

class GetEmbedInfo : public EmbedFilter
{
public:

    virtual void Selection(std::vector<DataSink>& t_hist, int t_entry) override
    {
         reco_p_ = track_->GetMomentum(); 
         if(embed_track_) cor_p_ = embed_track_->GetInitialMom();
         this->FillData(t_hist, t_entry);
    };

    std::vector<TString> TrackDescription()
    {
        auto embed_data = TString::Format("Reco info: |P| = %.1f, #theta = %.1f, #phi = %.1f", reco_p_.Mag(), reco_p_.Theta()*180/M_PI, reco_p_.Phi()*180/M_PI);
        if(embed_track_)
             return {embed_data, TString::Format("MC info: |P| = %.1f, #theta = %.1f, #phi = %.1f", 1e3*cor_p_.Mag(), cor_p_.Theta()*180/M_PI, cor_p_.Phi()*180/M_PI)};
        return {embed_data};
    }

    std::unique_ptr<Rule> Clone() override
    {
        return std::make_unique<GetEmbedInfo>(*this);
    };
    
protected:
    //std::shared_ptr<ReaderValue> myEmbedArray_;

    TVector3 reco_p_;
    TVector3 cor_p_;
};

void EmbedTrackPlotNoCompare(RuleTree& t_Additional_rules, bool t_show_rejected = false, const std::string& t_filename="data/Run2841_WithProton/TrackDistComp/Mom_350.0_450.0_Theta_10.0_20.0_Group_0/run2841_s*")
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

	RuleTree tree;
	tree.AppendRule<TrackZFilter>("min_track_num");
        tree.AppendRule<DrawTrack>("all_xz");
        tree.AppendRule<DrawTrack>("all_yz", 1, 2);
        tree.AppendRule<EmbedFilter>("EFilter");
        tree.AppendTree(t_Additional_rules);
        tree.AppendRule<DrawTrack>("E_xz");
        tree.AppendRule<DrawTrack>("E_yz", 1, 2);
        auto info = tree.AppendRule<GetEmbedInfo>("info")[0];
        tree.AppendRule<ShowEntry>("entry");
        
        tree.WireTap("all_xz");
        tree.WireTap("all_yz");
        tree.WireTap("E_xz");
        tree.WireTap("E_yz");


 	DrawMultipleComplex embed_draw(t_filename.c_str(),"cbmsim");
 
	embed_draw.SetRule(tree.first_rule_.get());


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
			const auto& result_embed = embed_datalist[i + 2];
			
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
                        for(auto& text : info->TrackDescription()) description.AddText(text.Data());
                        description.Draw("same");
			std::cout << "Continue ? \n";
			gPad->WaitPrimitive();
		}
	}

}
