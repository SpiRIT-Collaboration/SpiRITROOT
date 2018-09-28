//#include "/mnt/spirit/analysis/user/tsangc/create_submit.C"
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <TString.h>
#include <vector>
#include "ConfigListToConfig.C"
#include "run_general.C"


const std::map<std::string, int> pname2id{ // tables for particle name to pdg_table id
{"pi+", 211},
{"p", 2212},
{"d", 1000010020},
{"t", 1000010030},
{"3He", 1000020030},
{"4He", 1000020040},
{"6Li", 1000030060},
{"7Li", 1000030070},
};


std::vector<double> linspace(double t_min, double t_max, int t_num)
{
    /****************************************************************
    * returns a uniformly spaced vector with given number of points *
    *****************************************************************/
    std::vector<double> linspace(t_num, t_min);
    if(t_num == 1) return linspace;

    double dx = (t_max - t_min)/(double) (t_num - 1);
    double val = t_min;
    for(int i = 0; val <= t_max && i < t_num; val += dx, ++i)
        linspace[i] = val;
    return linspace;
}

void SplitVertexFiles(const TString& t_vertex_file, std::vector<std::string>& filenames)
{
    /*
    Split vertex files into different files according to their run number
    accepts the name of the original vertex file
    returns the list of run that is contained in the file
    */
    TString workDir   = gSystem -> Getenv("VMCWORKDIR");
    TString parDir    = workDir + "/parameters/";
    TString splitDir  = parDir + "VertexSplit/";

    // create split direcotory if non-existant
    system(("mkdir -p " + splitDir).Data());

    // open Justin's vertex file
    std::ifstream vertex((parDir + t_vertex_file).Data());
    if(!vertex.is_open())
        std::cerr << "Cannot find vertex file " << parDir + t_vertex_file << " for splitting\n";

    // get rid of header
    std::string header, line;
    std::getline(vertex, header);
    int current_run_num = 0;

    std::ofstream new_vertex;
    // split it into files each contains only 500 events
    int num_events = 501;

    // fill vector to return data
    filenames.clear();
    while(std::getline(vertex, line))
    {
        if(num_events > 500)
        {
            if(new_vertex.is_open()) new_vertex.close();

            std::string filename(TString::Format("%s/Vertex_Num_%04d", splitDir.Data(), current_run_num).Data());
            new_vertex.open(filename);
            filenames.push_back(filename);
            new_vertex << header << "\n";

            num_events = 0;
            ++current_run_num;
        }
	++num_events;
        new_vertex << line << "\n";
    }
}

void run_random()
{
    TString workDir   = gSystem -> Getenv("VMCWORKDIR");
    TString parDir    = workDir + "/parameters/";

    ConfigListIO configlist;
    configlist.SetHeader({"Filename", "RandomMomentum", "VertexFile", "Particle", "PhiLimit", "ThetaLimit"});

    std::vector<std::string> filename_list;
    SplitVertexFiles("Vertex_r2841.txt", filename_list);
    int part = 0;
    for(const auto& config_filename : filename_list)
    {
        configlist.SetElement("Filename", std::string(TString::Format("Test_Part%04d", part).Data()));
        ++part;
        configlist.SetElement("RandomMomentum", std::vector<double>{0.05, 0.2});
        configlist.SetElement("VertexFile", config_filename);
        configlist.SetElement("Particle", pname2id.at("pi+"));
        configlist.SetElement("PhiLimit", std::vector<double>{0, 1e-9});
        configlist.SetElement("ThetaLimit", std::vector<double>{-45, 45});
    
        std::cout << "Processing " << TString::Format("Run2841/Part%04d", part) << "\r";
        configlist.Fill();
    }
    std::cout << "\n";

    std::string list_name("pion_pixel.dat");
    configlist.ToText(list_name);
    run_general(list_name);

}
