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

void SplitVertexFiles(const TString& t_vertex_file, std::vector<int>& run_list, std::vector<std::string>& filenames)
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
    // fill vector to return data
    run_list.clear();
    filenames.clear();
    while(std::getline(vertex, line))
    {
        std::stringstream ss(line);
        int run_num;
        ss >> run_num;
        if(run_num != current_run_num)
        {
            current_run_num = run_num;
            run_list.push_back(run_num);
            if(new_vertex.is_open()) new_vertex.close();

            std::string filename(TString::Format("%s/Vertex_Run_%04d", splitDir.Data(), run_num).Data()); 
            new_vertex.open(filename);
            filenames.push_back(filename);
            new_vertex << header << "\n";
        }
        new_vertex << line << "\n";
    }
}

void run_pion_pixel()
{
    TString workDir   = gSystem -> Getenv("VMCWORKDIR");
    TString parDir    = workDir + "/parameters/";

    // set pixel size
    const double phi_min = -180;
    const double phi_max = 180;
    const int phi_num_pixel = 10;

    const double theta_min = 0.;
    const double theta_max = 30.;
    const int theta_num_pixel = 5;

    const double momentum_min = 350; // MeV / c
    const double momentum_max = 360;
    const int momentum_num = 1;

    ConfigListIO configlist;
    configlist.SetHeader({"Filename", "Momentum", "VertexFile", "Particle", "Phi", "Theta"});

    int index = 0;
    std::vector<int> num_list;
    std::vector<std::string> filename_list;
    SplitVertexFiles("VertexLocation_real.txt", num_list, filename_list);
    for(const auto& momentum : linspace(momentum_min, momentum_max, momentum_num))
        for(const auto& phi : linspace(phi_min, phi_max, phi_num_pixel))
            for(const auto& theta : linspace(theta_min, theta_max, theta_num_pixel))
            {
                for(int i = 0; i < 1/*num_list.size()*/; ++i)
                {
                    int num = 2841;//num_list[i];
                    std::string config_filename= filename_list[i];

                    configlist.SetElement("Filename", std::string(TString::Format("PionPixel/Run_%04d/PionPixel_ID_%04d_Momentum_%3.2f", num, index, momentum).Data()));
                    configlist.SetElement("Momentum", momentum/1.e3);
                    configlist.SetElement("VertexFile", "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/parameters/Vertex_r2841.txt"/*config_filename*/);
                    configlist.SetElement("Particle", pname2id.at("pi+"));
                    configlist.SetElement("Phi", phi);
                    configlist.SetElement("Theta", theta);

                    std::cout << "Processing " << TString::Format("PionPixel/Run_%04d/PionPixel_ID_%04d_Momentum_%3.2f", num, index, momentum) << "\r";
                    configlist.Fill();
                }
		++index;
            }
    std::cout << "\n";

    std::string list_name("pion_pixel.dat");
    configlist.ToText(list_name);
    run_general(list_name);

}
