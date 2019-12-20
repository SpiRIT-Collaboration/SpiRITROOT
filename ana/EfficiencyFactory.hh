#ifndef EFFICIENCYFACTORY
#define EFFICIENCYFACTORY
#include <vector>
#include <utility>
#include <string>
#include <iostream>

#include "TObject.h"
#include "TEfficiency.h"
#include "TFile.h"
#include "TTree.h"
#include "TTreeReader.h"

class EfficiencyFactory
{
public:
    struct Binning{double min; double max; int nbins;};
    EfficiencyFactory(){};
    virtual ~EfficiencyFactory(){};
    /*
    * Assorted Setters to change cut conditions
    */
    EfficiencyFactory& SetMomBins(double t_min, double t_max, int t_bins);
    EfficiencyFactory& SetThetaBins(double t_min, double t_max, int t_bins); 
    EfficiencyFactory& SetPhiBins(double t_min, double t_max, int t_bins); 
    EfficiencyFactory& SetPhiCut(const std::vector<std::pair<double, double>>& t_phi_cut);
    EfficiencyFactory& SetTrackQuality(int t_nclus, double t_dpoca);

    virtual TEfficiency FinalizeBins(int t_pdg,
                                     bool t_verbose=false) = 0;

protected:
    double dist_2_vert_;
    int num_clusters_;
    std::vector<std::pair<double, double>> phi_cut_;
    Binning mom_bin_{50, 600, 30};
    Binning theta_bin_{0, 90, 45};
};

class OrigEfficiencyFactory : public EfficiencyFactory
{
public:  
    OrigEfficiencyFactory(const std::string& t_efficiency_db);
    ~OrigEfficiencyFactory(){};
    virtual TEfficiency FinalizeBins(int t_pdg,
                                     bool t_verbose=false);
protected:
    std::string data_filename_;
};

class EfficiencyFromAnaFactory : public EfficiencyFactory
{
public:
    EfficiencyFromAnaFactory();
    virtual ~EfficiencyFromAnaFactory(){};
    void SetDataBaseForPDG(int t_pdg, const std::string& t_efficiency_db);
    
    virtual TEfficiency FinalizeBins(int t_pdg,
                                     bool t_verbose=false); 

protected:
    std::map<int, std::string> fEfficiencyDB;
};

class EfficiencyFromConcFactory : public EfficiencyFactory
{
public:
    EfficiencyFromConcFactory();
    virtual ~EfficiencyFromConcFactory(){};
    void SetDataBaseForPDG(int t_pdg, const std::string& t_efficiency_db);
    
    virtual TEfficiency FinalizeBins(int t_pdg,
                                     bool t_verbose=false); 

protected:
    std::map<int, std::string> fEfficiencyDB;
};
#endif
