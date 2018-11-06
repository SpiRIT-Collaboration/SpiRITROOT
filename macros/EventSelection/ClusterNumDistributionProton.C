#include "EventSelection/ChooseFolder.h"


void ClusterNumDistribution(double mom_min = 300, double mom_max = 350, double theta_min = 10, double theta_max = 20)
{
	std::cout << mom_min << " " << mom_max << " " << theta_min << " " << theta_max << "\n";
	auto filename = AggrateFolder(mom_min, mom_max, theta_min, theta_max);
	std::cout << "Reading from :\n"; 
	TChain chain("cbmsim");
	for(const auto& name : filename)
	{
		std::cout << name << std::endl;
		chain.Add(name.c_str());
	}

	DrawMultipleComplex mc_draw(&chain);
	auto cp = mc_draw.NewCheckPoints(5);

	TrackZFilter min_track_num;//([](int i){return i > 3;});
	EmbedExistence existence;
	EmbedFilter filter;
	MomentumTracks embed_mom(3);
	ValueCut mom_cut(mom_min, mom_max);
	ClusterNum embed_cnum, rejected_cnum;

	// plot real protons
	PID pid1, pid2, pid3;
	EmbedCut pid_cut("ProtonCut.root");
	ThetaPhi theta_phi;
	ValueCut theta_cut(theta_min, theta_max);
 
        min_track_num.AddRule(existence.AddRule(
                              filter.AddRule(
                              pid1.AddRule(
                              cp[0]->AddRule(
                              theta_phi.AddRule(
	                      theta_cut.AddRule(
                              pid2.AddRule(
                              cp[1]->AddRule(
	                      embed_mom.AddRule(
	                      mom_cut.AddRule(
                              pid3.AddRule(
                              cp[2]->AddRule(
                              pid_cut.AddRule(
                              embed_cnum.AddRule(cp[3])))))))))))))));

	pid_cut.AddRejectRule(rejected_cnum.AddRule(cp[4]));

	TH1F embed_hist("embed", "", 100, 0, 100);
	TH1F rejected_embed("rejected_embed", "", 100, 0, 100);
	TH2F pid_hist("Embeded_PID", "", 100, 0, 1500, 100, 0, 1000);
        TH2F pid_hist_theta("Embeded_PID_Theta_cut", "", 100, 0, 1500, 100, 0, 1000);
        TH2F pid_hist_theta_mom("Embeded_PID_Theta_mom_cut", "", 100, 0, 1500, 100, 0, 1000);
	mc_draw.DrawMultiple(min_track_num, pid_hist, pid_hist_theta, pid_hist_theta_mom, embed_hist, rejected_embed);

	DailyLogger logger(std::string(TString::Format("Mom_%.1f_%.1f_Theta_%.1f_%.1f", mom_min, mom_max, theta_min, theta_max).Data()));
	logger.SaveClass(embed_hist);
	logger.SaveClass(rejected_embed);
        logger.SaveClass(pid_hist);
        logger.SaveClass(pid_hist_theta);
        logger.SaveClass(pid_hist_theta_mom);
}

void ClusterNumDistributionProton()
{
  std::vector<double> mom_range{300, 350, 400, 450};
  std::vector<double> angle_range{0, 10, 20, 30, 40};
  for(int i = 0; i < mom_range.size()-1; ++i)
    for(int j = 0; j < angle_range.size()-1; ++j)
    {
      //if(i == 0 && j == 0 ) continue; // for some unknown reason, data for 300 < p < 350 and 0 < theta < 10 is missing. Will skip
      ClusterNumDistribution(mom_range[i], mom_range[i+1], angle_range[j], angle_range[j+1]);
    }
}
