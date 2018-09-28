//#include "/mnt/spirit/analysis/user/tsangc/create_submit.C"
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <TString.h>
#include <vector>
#include <dirent.h>


const int num_jobs_in_queue = 20; // only allow this amount of jobs on ember for other users

bool hasEnding (std::string const &fullString, std::string const &ending) 
{
	if (fullString.length() >= ending.length()) 
	{
		return (0 == fullString.compare (fullString.length() - ending.length(), ending.length(), ending));
	} else {
		return false;
	}
}

const std::string t_dir = "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/data/Cocktail";

void run_reco_ember(int t_start_from = 1, int t_num_jobs_to_be_submitted = num_jobs_in_queue)
{
	// count the job order
	// i.e. first job has job_order = 1, second has job_order = 2....
	// we will submit jobs only when job_order > t_start_from
	// this is to make sure that when 1 job ends, it will call other jobs to maintain num_jobs_in_queue
	int job_order = 0;

	// loop through all the files in the directory and create name list of root files to be reconstructed
	// number of events is needed in the run_reco_mc script, so it will be stored together
	std::vector<std::pair<std::string, unsigned>> fileinfos;
	DIR *dpdf;
	struct dirent *epdf;

	dpdf = opendir(t_dir.c_str());
	if(dpdf != NULL)
		while((epdf = readdir(dpdf)))
		{
			std::string fullname(epdf->d_name);
			const std::string digi_suffix(".mc.root");
			if(hasEnding(fullname, digi_suffix))
			{
				// find the number of events
				TFile file((t_dir + "/" + fullname).c_str());
				auto tree = (TTree*) file.Get("cbmsim");
				unsigned entries = tree->GetBranch("PrimaryTrack")->GetEntries();
				
				fileinfos.push_back({fullname.substr(0, fullname.size() - digi_suffix.size()), entries});
			}
		}

	for(const auto& info : fileinfos)
	{
		++job_order;
		if(job_order < t_start_from)
			continue;

		// end the script if too many jobs are submitted
		if(job_order - t_start_from >= t_num_jobs_to_be_submitted)
			return;	

		// skip file if it exists
		TFile file((info.first + ".reco.root").c_str());
		if(file.IsOpen())
		{
			std::cout << info.first << " exist. We will skip to the next job\n";
			run_reco_ember(t_start_from + num_jobs_in_queue, t_num_jobs_to_be_submitted);
			return;
		}

		// right now we assume number of produced particles = total num / 6 as 6 being the numbers of available cocktail particles
		// start simulation
		std::cout << "Start reconstruction with output " << info.first << "\n";
		TString command = TString::Format("sbatch ./submit_reco.sh \"Cocktail/%s\" %d %d", info.first.c_str(), info.second, job_order + num_jobs_in_queue);

		std::cout << " With command " << command << "\n";
		system(command.Data());
		std::cout << "Job submitted!\n";
	}
}
