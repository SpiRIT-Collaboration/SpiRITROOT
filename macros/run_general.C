//#include "/mnt/spirit/analysis/user/tsangc/create_submit.C"
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <TString.h>
#include <vector>

struct RunInfo{
	std::string filename;
	int nevent;
	double momentum;
	int particle;
	double phi, theta;
};

void RunListToFile(const std::vector<RunInfo>& t_run_list, const std::string& t_filename)
{
	std::ofstream file(t_filename.c_str());
	if(!file.is_open())
	{
		std::cerr << "Cannot write to file " << t_filename << "\n";
		return;
	}

	// header for people to read
	file << "#";
	for(const auto& name : std::vector<std::string>{"Filename", "NEvent", "Momentum", "Particle", "Phi", "Theta"})
		file << std::setw(20) << name;
	file << "\n";

	// write everything
	for(const auto& info : t_run_list)
	{
		file << std::setw(20) << info.filename;
		file << std::setw(20) << info.nevent;
		file << std::setw(20) << info.momentum;
		file << std::setw(20) << info.particle;
		file << std::setw(20) << info.phi;
		file << std::setw(20) << info.theta;
		file << "\n";
	}
}

std::vector<RunInfo> RunListFromFile(const std::string& t_filename)
{
	std::vector<RunInfo> info_list;

	std::ifstream file(t_filename.c_str());
	if(!file.is_open())
	{
		std::cerr << "Run list file " << t_filename << " cannot be opened\n";
		return info_list;
	}

	std::string line;
	while(std::getline(file, line))
	{
	        // erase everything after # char
	        auto pos = line.find("#");
	        if(pos != std::string::npos)
	                line.erase(line.begin() + pos, line.end());
	
	        const auto first_char = line.find_first_not_of(" \t\r\n");
	        if(first_char == std::string::npos) // skip if the line is empty
	                continue;

		RunInfo info;
		std::stringstream ss(line);
		if(!(ss >> info.filename >> info.nevent >> info.momentum >> info.particle >> info.phi >> info.theta))
		{
			std::cerr << "Cannot read line " << line << "\n";
			continue;
		}

		info_list.push_back(info);
	}

	return info_list;
	
}

const int num_jobs_in_queue = 10; // only allow this amount of jobs on ember for other users

void run_general(const std::string& t_config_list, int t_start_from = 1, int t_num_jobs_to_be_submitted = num_jobs_in_queue)
{
	// count the job order
	// i.e. first job has job_order = 1, second has job_order = 2....
	// we will submit jobs only when job_order > t_start_from
	// this is to make sure that when 1 job ends, it will call other jobs to maintain num_jobs_in_queue
	int job_order = 0;

	for(const auto& info : RunListFromFile(t_config_list))
	{
		++job_order;
		if(job_order < t_start_from)
			continue;

		// end the script if too many jobs are submitted
		if(job_order - t_start_from >= t_num_jobs_to_be_submitted)
			return;	

		// name of the output mc files
		TString output_name(info.filename);

		// skip file if it exists
		TFile file(("data/" + output_name + ".digi.root").Data());
		if(file.IsOpen())
		{
			std::cout << output_name << " exist. We will skip to the next job\n";
			run_general(t_config_list, t_start_from + num_jobs_in_queue, t_num_jobs_to_be_submitted);
			return;
		}

		// right now we assume number of produced particles = total num / 6 as 6 being the numbers of available cocktail particles
		// start simulation
		std::cout << "Start simulation with output " << output_name << "\n";
		TString command = TString::Format("sbatch ./submit_general.sh \"%s\" %d %f %d %f %f %d \"%s\"", output_name.Data(), info.nevent, info.momentum, info.particle, info.phi, info.theta, job_order + num_jobs_in_queue, t_config_list.c_str());

		std::cout << " With command " << command << "\n";
		system(command.Data());
		std::cout << "Job submitted!\n";
	}

}
