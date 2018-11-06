#ifndef CHOOSEFOLDER_H
#define CHOOSEFOLDER_H

std::string DetermineFolder(double mom_min, double mom_max, double theta_min, double theta_max)
{
	int mom_index = (int) ((mom_min - 300.)/50.);
	int theta_index = (int) (theta_min/10.);
	mom_min = 300. + (double) mom_index*50.;
	mom_max = mom_min + 50.;
	theta_min = (double) theta_index*10.;
	theta_max = theta_min + 10.;
	return std::string(TString::Format("data/Run2841_WithProton/TrackDistComp/Mom_%.1f_%.1f_Theta_%.1f_%.1f/run2841_s*.reco.tommy_branch.1749.ef952af.root", mom_min, mom_max, theta_min, theta_max).Data());
}

std::vector<std::string> AggrateFolder(double mom_min, double mom_max, double theta_min, double theta_max, const std::string t_file_end="/run2841_s*.reco.tommy_branch.1749.ef952af.root")
{
	int mom_min_index = (int) ((mom_min - 300.)/50.);
	int theta_min_index = (int) (theta_min/10.);
	int mom_max_index = (int) ((mom_max - 300.)/50.);
	int theta_max_index = (int) (theta_max/10.);

	if(mom_min_index == mom_max_index) ++mom_max_index;
	if(theta_min_index == theta_max_index) ++theta_max_index;

	std::vector<std::string> name_list;
	for(int i = theta_min_index; i < theta_max_index; ++i)
		for(int j = mom_min_index; j < mom_max_index; ++j)
		{
			mom_min = 300. + (double) j*50.;
			mom_max = mom_min + 50.;
			theta_min = (double) i*10.;
			theta_max = theta_min + 10.;
			name_list.push_back(TString::Format("data/Run2841_WithProton/TrackDistComp/Mom_%.1f_%.1f_Theta_%.1f_%.1f%s", mom_min, mom_max, theta_min, theta_max, t_file_end.c_str()).Data());
		}
	return name_list;
}

#endif
