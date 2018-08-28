//#include "/mnt/spirit/analysis/user/tsangc/create_submit.C"
#include <map>
#include <iostream>
#include <string>
#include <sstream>
#include <TString.h>
#include <vector>
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

std::vector<int> SplitVertexFiles(const TString& t_vertex_file="VertexLocation_real.txt")
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
    std::vector<int> run_list;
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

            new_vertex.open(TString::Format("%s/Vertex_Run_%04d", splitDir.Data(), run_num).Data());
            new_vertex << header << "\n";
        }
        new_vertex << line << "\n";
    }
    
    return run_list;
}

void run_pion_pixel()
{
    TString workDir   = gSystem -> Getenv("VMCWORKDIR");
      TString parDir    = workDir + "/parameters/";

    // set pixel size
    const double phi_min = 0.;
    const double phi_max = 360.;
    const int phi_num_pixel = 2;

    const double theta_min = 5.;
    const double theta_max = 90.;
    const int theta_num_pixel = 2;

    const double momentum_min = 100; // MeV / c
    const double momentum_max = 600;
    const int momentum_num = 2;

    std::vector<RunInfo> info_list;

    int index = 0;
    auto num_list = SplitVertexFiles();
    for(const auto& momentum : linspace(momentum_min, momentum_max, momentum_num))
        for(const auto& phi : linspace(phi_min, phi_max, phi_num_pixel))
            for(const auto& theta : linspace(theta_min, theta_max, theta_num_pixel))
            {
                for(const auto& num : num_list)
                {
                    RunInfo info;
                    info.filename = std::string(TString::Format("PionPixel/Run_%04d/PionPixel_ID_%04d_Momentum_%3.2f", num, index, momentum).Data());
                    info.momentum = momentum/1.e3;
                    info.vertexfile = std::string(TString::Format("%s/VertexSplit/Vertex_Run_%04d", parDir.Data(), num).Data());
                    info.particle = pname2id.at("pi+");
                    info.phi = phi;
                    info.theta = theta;

                    std::cout << "Processing " << info.filename << "\r";
                    info_list.push_back(info);
                }
		++index;
            }
    std::cout << "\n";

    std::string list_name("pion_pixel.dat");
    RunListToFile(info_list, list_name);
    run_general(list_name);

}
