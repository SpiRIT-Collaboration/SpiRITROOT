#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <TString.h>
#include <vector>
#include "ConfigListToConfig.C"

const int num_jobs_in_queue = 20; // only allow this amount of jobs on ember for other users
const int NTOTAL=10000;
const std::string FISHTANK_USER = "tsangc";
const std::string DESTINATION = "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/data";

void CreateTempAndAllowExec(char *t_tempname, const std::string& t_template, const TString& t_logdir, const TString& t_workdir)
{
  int fd = mkstemp(t_tempname);
  TString command = TString::Format("sed \'s/LOGDIRTOBESUB/%s/g\' %s/macros/embedMacros/%s > %s", t_logdir.Data(), t_workdir.Data(), t_template.c_str(), t_tempname);
  system(command.Data());
  command = TString::Format("chmod 755 %s", t_tempname);
  system(command.Data());
  close(fd);
}

void run_general_all(const std::string& t_config_list, int t_start_from = 0, int t_num_jobs_to_be_submitted = num_jobs_in_queue)
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

    // create the parameter files
    std::string par_name(".temp_");
    par_name = par_name + std::to_string(i);
    config_list.ToConfig(i, std::string(parDir.Data()) + par_name);


    // right now we assume number of produced particles = total num / 6 as 6 being the numbers of available cocktail particles
    // start simulation
    std::cout << "Start simulation with output " << output_name << "\n";
    // copy and past logdir into submit_general
    // the only way to get slurm log file to store in a designated area
    char tempname4reco[] = "/tmp/fileXXXXXX";
    char tempname4mc[] = "/tmp/fileXXXXXX";
    char tempname4cleanup[] = "/tmp/fileXXXXXX";  

    logdir.ReplaceAll("/", "\\/");
    CreateTempAndAllowExec(tempname4reco, "submit_general_allreco.sh", logdir, workDir);
    CreateTempAndAllowExec(tempname4mc, "submit_general_allmc.sh", logdir, workDir);
    CreateTempAndAllowExec(tempname4cleanup, "cleanup.sh", logdir, workDir);


    command = TString::Format("%s/macros/embedMacros/submit_general_all.sh \"%s\" \"%s\" %d \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" \"%s\" %d",
                workDir.Data(),
                tempname4reco,
                output_name.Data(),
                i + num_jobs_in_queue,
                t_config_list.c_str(),
                input_name.Data(),
                DESTINATION.c_str(),
                FISHTANK_USER.c_str(),
                par_name.c_str(),
                tempname4mc,
                tempname4cleanup,
                NTOTAL);

    std::cout << " With command " << command << "\n";
    system(command.Data());
    std::cout << "Job submitted!\n";
    // delete temp file
    unlink(tempname4reco);
    unlink(tempname4mc);
    unlink(tempname4cleanup);
  }

}
