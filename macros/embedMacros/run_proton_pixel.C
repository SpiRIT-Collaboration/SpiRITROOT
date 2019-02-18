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


void run_proton_pixel()
{
    TString workDir   = gSystem -> Getenv("VMCWORKDIR");
    TString parDir    = workDir + "/parameters/";

    // set pixel size
    const std::vector<double> theta{0, 10, 20, 30, 40};
    const std::vector<double> momentum{300, 350, 400, 450};

    ConfigListIO configlist;
    configlist.SetHeader({"Filename", "RandomMomentum", "VertexFile", "Particle", "PhiLimit", "ThetaLimit"});

    for(unsigned i = 0; i < theta.size() - 1; ++i)
        for(unsigned j = 0; j < momentum.size() - 1; ++j)
        {
            configlist.SetElement("Filename", std::string(TString::Format("Run2841_WithProton/TrackDistComp/Mom_%.1f_%.1f_Theta_%.1f_%.1f", momentum[j], momentum[j+1], theta[i], theta[i+1]).Data()));
            configlist.SetElement("RandomMomentum", std::vector<double>{momentum[j]/1.e3, momentum[j+1]/1.e3});
            configlist.SetElement("VertexFile", "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/parameters/Vertex_r2841.txt");
            configlist.SetElement("Particle", pname2id.at("p"));
            configlist.SetElement("PhiLimit", std::vector<double>{0, 360});
            configlist.SetElement("ThetaLimit", std::vector<double>{theta[i], theta[i+1]});

            std::cout << "Processing " << TString::Format("Run2841_WithProton/TrackDistComp/Mom_%.1f_%.1f_Theta_%.1f_%.1f", momentum[j], momentum[j+1], theta[i], theta[i+1])<< "\n";
            configlist.Fill();
        }
    std::cout << "\n";

    std::string list_name("pion_pixel.dat");
    configlist.ToText(list_name);
    run_general(list_name);

}
