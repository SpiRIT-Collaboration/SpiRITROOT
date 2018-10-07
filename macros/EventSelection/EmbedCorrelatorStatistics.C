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


void EmbedCorrelatorStatistics()
{
	DrawComplex mc_draw("data/Run2841_WithOffset/HighEnergy/Run_2841_full.reco.mc.root", "cbmsim");//HighEnergy/Run_2841_full.reco.mc.root", "cbmsim");
	DrawComplex embed_draw("data/Run2841_WithOffset/HighEnergy/run2841_s*.reco.develop.1737.f55eaf6.root", "cbmsim");

	RecoTrackNumFilter track_num_filter;
	DrawHit hit;
	track_num_filter.AddRule(&hit);

	TrackZFilter min_track_num;//([](int i){return i > 3;});
	EmbedFilter filter;
	DrawTrack track;
	TrackShapeFilter shape_filter("HitsHighECutG.root", 0.8);
 
        min_track_num.AddRule(filter.AddRule(
                              track.AddRule(
                              &shape_filter)));
                              

	EntryRecorder num_multiple_mc, num_empty_embed, num_shape_mismatch;

	track_num_filter.AddRejectRule(&num_multiple_mc);
	min_track_num.AddRejectRule(&num_empty_embed);
	shape_filter.AddRule(&num_shape_mismatch);

	mc_draw.FillRule<TH1F>(track_num_filter, "TEMP", "T", 100, 0, 100);
	embed_draw.FillRule<TH1F>(min_track_num, "TEMP2", "", 100, 0, 100);
	std::cout << "\n";

	
	auto list_shape_mismatch = num_shape_mismatch.GetList();
	auto list_multiple_mc = num_multiple_mc.GetList();
	std::vector<double> intersect(list_shape_mismatch.size() + list_multiple_mc.size());
	auto it = std::set_intersection( list_shape_mismatch.begin(), list_shape_mismatch.end(), list_multiple_mc.begin(), list_multiple_mc.end(), intersect.begin());
	intersect.resize(it - intersect.begin());

	std::cout << " Shape mismatch " << list_shape_mismatch.size() << "\n";
	std::cout << " Shape mismatch due to multiple MC tracks " << intersect.size() << "\n";
	std::cout << " Empty Embeded tracks " << num_empty_embed.GetList().size() << "\n";
	num_empty_embed.ToFile("ListEmpty.dat");

}
