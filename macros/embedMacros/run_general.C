//#include "/mnt/spirit/analysis/user/tsangc/create_submit.C"
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <TString.h>
#include <vector>
#include "ConfigListToConfig.C"

const int num_jobs_in_queue = 50; // only allow this amount of jobs on ember for other users

void run_general(const std::string& t_config_list, int t_start_from = 0, int t_num_jobs_to_be_submitted = num_jobs_in_queue)
{
	TString workDir   = gSystem -> Getenv("VMCWORKDIR");
  	TString parDir    = workDir + "/parameters/";

	ConfigListIO config_list;
	config_list.FillFromText(t_config_list);

	for(unsigned i = t_start_from; i < config_list.Size(); ++i)
	{
		// end the script if too many jobs are submitted
		if(i - t_start_from >= t_num_jobs_to_be_submitted)
			return;	

		// name of the output mc files
		TString output_name(config_list.GetElement(i, "Filename"));

		// create the parameter files
		std::string par_name(".temp_");
		par_name = par_name + std::to_string(i);
		config_list.ToConfig(i, std::string(parDir.Data()) + par_name);

		// skip file if it exists
		TFile file(("data/" + output_name + ".digi.root").Data());
		if(file.IsOpen())
		{
			std::cout << output_name << " exist. We will skip to the next job\n";
			//run_general(t_config_list, t_start_from + num_jobs_in_queue, t_num_jobs_to_be_submitted);
			//return;
		}

		// right now we assume number of produced particles = total num / 6 as 6 being the numbers of available cocktail particles
		// start simulation
		std::cout << "Start simulation with output " << output_name << "\n";
             
		TString command = TString::Format("sbatch %s/macros/embedMacros/submit_general.sh \"%s\" %d \"%s\" \"%s\"", workDir.Data(), output_name.Data(), i + num_jobs_in_queue, t_config_list.c_str(), par_name.c_str());

		std::cout << " With command " << command << "\n";
		system(command.Data());
		std::cout << "Job submitted!\n";
	}

}
