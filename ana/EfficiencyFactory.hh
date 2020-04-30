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
#include "TH2.h"
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
    EfficiencyFactory& SetPhiBins(double t_min, double t_max, int t_bins); 
    EfficiencyFactory& SetPhiCut(const std::vector<std::pair<double, double>>& t_phi_cut);
    EfficiencyFactory& SetTrackQuality(int t_nclus, double t_dpoca);

    // range setters for Lab frame
    EfficiencyFactory& SetMomBins(double t_min, double t_max, int t_bins);
    EfficiencyFactory& SetThetaBins(double t_min, double t_max, int t_bins); 

    // range setters that are only used when the efficiency is in CM frame
    EfficiencyFactory& SetPtBins(double t_min, double t_max, int t_bins); 
    EfficiencyFactory& SetCMzBins(double t_min, double t_max, int t_bins);


    virtual TEfficiency FinalizeBins(int t_pdg,
                                     bool t_verbose=false) = 0;
    virtual bool IsInCM() { return false; };
    virtual void SetUnfoldingDist(TH2F* dist) {};

    // additional factor for phi phase space coverage
    void SetPhaseSpaceFactor(double t_factor) { phase_space_factor = t_factor; };
    void SetUpScalingFactor(double t_factor) { upscale_factor = t_factor; };
protected:
    TH2F* UpScaling(TH2F* t_hist, double t_xfactor, double t_yfactor);

    double dist_2_vert_;
    int num_clusters_;
    std::vector<std::pair<double, double>> phi_cut_;
    Binning mom_bin_{50, 600, 30};
    Binning theta_bin_{0, 90, 45};
    Binning pt_bin_{0, 2500, 10};
    Binning CMz_bin_{-1500, 1500, 10};
    double phase_space_factor = 1;
    double upscale_factor = 1;

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
    static void CompressFile(const std::string& t_efficiency_db, const std::string& t_compressed_db);  
    virtual TEfficiency FinalizeBins(int t_pdg,
                                     bool t_verbose=false); 
    virtual void SetUnfoldingDist(TH2F* dist);
protected:
    std::map<int, std::string> fEfficiencyDB;
    TH2F *fUnfoldingDist = nullptr;
};

class EfficiencyInCMFactory : public EfficiencyFactory
{
public:
    EfficiencyInCMFactory();
    virtual ~EfficiencyInCMFactory(){};
    void SetDataBaseForPDG(int t_pdg, const std::string& t_efficiency_db);
    virtual TEfficiency FinalizeBins(int t_pdg,
                                     bool t_verbose=false); 

    static void TransformBackToCM(const std::string& t_efficiency_db, const std::string& t_cm_db, int fragMass, int targetMass, int charge, double energyPerN);
    virtual bool IsInCM() { return true; }
    virtual void SetUnfoldingDist(TH2F* dist);
protected:
    std::map<int, std::string> fEfficiencyDB;
    TH2F *fUnfoldingDist = nullptr;
};
#endif
