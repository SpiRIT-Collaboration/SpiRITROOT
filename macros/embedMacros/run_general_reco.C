#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <TString.h>
#include <vector>
#include "ConfigListToConfig.C"

const int num_jobs_in_queue = 4; // only allow this amount of jobs on ember for other users
const std::string FISHTANK_USER = "tsangc";
const std::string DESTINATION = "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/data";

void run_general_reco(const std::string& t_config_list, int t_start_from = 0, int t_num_jobs_to_be_submitted = num_jobs_in_queue)
{
	TString workDir   = gSystem -> Getenv("VMCWORKDIR");
  	TString parDir    = workDir + "/parameters/";

	// test internet connection between fishtank and hpcc first
	TString command = TString::Format("%s/macros/embedMacros/test_upload.sh %s %s", workDir.Data(), DESTINATION.c_str(), FISHTANK_USER.c_str());
	system(command.Data());



	ConfigListIO config_list;
	config_list.FillFromText(t_config_list);

	for(unsigned i = t_start_from; i < config_list.Size(); ++i)
	{
		// end the script if too many jobs are submitted
		if(i - t_start_from >= t_num_jobs_to_be_submitted)
			return;	

		// name of the input mc files
		TString input_name(config_list.GetElement(i, "Filename"));

		// name of the output reco files
		TString output_name(input_name);// + "_WithBDC");

		// directory for logging
		// originally it should be created in submit_general_reco.sh
		// but slurm log files dictates that this folder must be created before job submission
		// that's why it mush be created now
		auto logdir = workDir + "/macros/log/" + output_name;
		gSystem->mkdir(logdir.Data(), true);


		// right now we assume number of produced particles = total num / 6 as 6 being the numbers of available cocktail particles
		// start simulation
		std::cout << "Start simulation with output " << output_name << "\n";
		// copy and past logdir into submit_general
		// the only way to get slurm log file to store in a designated area
		char tempname[] = "/tmp/fileXXXXXX";
		int fd = mkstemp(tempname);
		logdir.ReplaceAll("/", "\\/");
		TString command = TString::Format("sed \'s/LOGDIRTOBESUB/%s/g\' %s/macros/embedMacros/submit_general_reco.sh > %s", logdir.Data(), workDir.Data(), tempname);
		system(command.Data());
		command = TString::Format("chmod 755 %s", tempname);
		system(command.Data());
		close(fd);

		command = TString::Format("sbatch %s \"%s\" %d \"%s\" \"%s\" \"%s\" \"%s\"  --export=VMCWORKDIR=$VMCWORKDIR", tempname, output_name.Data(), i + num_jobs_in_queue, t_config_list.c_str(), input_name.Data(), DESTINATION.c_str(), FISHTANK_USER.c_str());

		std::cout << " With command " << command << "\n";
		system(command.Data());
		std::cout << "Job submitted!\n";
		// delete temp file
		// unlink(tempname);
		break;
	}

}
