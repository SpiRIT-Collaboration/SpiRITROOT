#ifndef CHOOSEFOLDER_H
#define CHOOSEFOLDER_H
#include <string>
#include <vector>

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

std::vector<std::string> AggrateFolder(double mom_min, double mom_max, 
                                       double theta_min, double theta_max, 
                                       double phi_min, double phi_max, 
                                       bool t_bdc = false,
                                       const std::string t_file_end="/run2841_s*.reco.tommy_branch.1749.ef952af.root")
{
    std::string dir = "data/Run2841_WithProton/TrackDistComp/";
    std::vector<std::string> folders;
    for(auto& filename : listdir(dir))
    { 
        if(IsDirectory(dir + filename))
        {
            std::istringstream iss(filename);
            // filename should be separated by _
            std::string value;

            bool within_range = true;
            bool is_bdc = false;
            bool contain_para = false;
            // we will test and see if the file is inside the desired parameter range
            auto GetRange = [&iss](double& t_lower, double& t_upper)
            {
                std::string value;
                std::getline(iss, value, '_');
                t_lower = std::stof(value);
                std::getline(iss, value, '_');
                t_upper = std::stof(value);
            };

            while(std::getline(iss, value, '_'))
            {
                double lower, upper;
                if(value == "Mom")
                {
                    GetRange(lower, upper);
                    if(lower >= mom_max || upper <= mom_min) within_range = false;
                    contain_para = true;
                }
                else if(value == "Theta")
                {
                    GetRange(lower, upper);
                    if(lower >= theta_max || upper <= theta_min) within_range = false;
                    contain_para = true;
                }
                else if(value == "Phi")
                {
                    GetRange(lower, upper);
                    if(lower >= phi_max || upper <= phi_min) within_range = false;
                    contain_para = true;
                }
                else if(value == "WithBDC") is_bdc = true;;
            }
            if(within_range && (is_bdc == t_bdc) && contain_para) folders.push_back(dir + filename + t_file_end);
        }
    } 	
    return folders;
}

#endif
