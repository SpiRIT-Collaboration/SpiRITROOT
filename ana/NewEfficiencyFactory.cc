#include "TChain.h"
#include "EfficiencyFactory.hh"
#include "ST_ClusterNum_DB.hh"

EfficiencyFactory::EfficiencyFactory(int t_num_clusters,
                                     double t_dist_2_vert,
                                     int t_min_mult,
                                     int t_max_mult,
                                     const std::vector<std::pair<double, double>>& t_phi_cut,
                                     const std::string& t_particle_name,
                                     const std::vector<std::pair<double, double>>& t_phi_range,
                                     const std::string& t_efficiency_db,
                                     const std::string& t_cut_db,
                                     double t_cluster_ratio) : dist_2_vert_(t_dist_2_vert),
                                                               num_clusters_(t_num_clusters),
                                                               min_mult_(t_min_mult),
                                                               max_mult_(t_max_mult),
                                                               phi_cut_(t_phi_cut),
                                                               phi_range_(t_phi_range), 
                                                               data_filename_(t_efficiency_db), 
                                                               cluster_ratio_(t_cluster_ratio)
    {
      if(t_particle_name == "pi+") treename_ = "pi+";
      else if(t_particle_name == "pi-") treename_ = "pi-";
      else if(t_particle_name == "proton") treename_ = "proton";
      else if(t_particle_name == "deuteron") treename_ = "deuteron";
      else if(t_particle_name == "triton") treename_ = "triton";
      else if(t_particle_name == "he3") treename_ = "he3";
      else throw std::runtime_error("EfficiencyFactory: Only pi+ and pi- are supported, not " + t_particle_name);

      if(!t_cut_db.empty()) std::cout << "Warning: PID Cut is no longer supported. The supplied PID TCutG will be ignored." << std::endl;
      // default: no phi cut
      if(phi_cut_.size() == 0) phi_cut_.push_back({0., 360.});
      if(phi_range_.size() == 0) phi_range_.push_back({0, 360});
    };

    /*
    * Assorted Setters to change cut conditions
    */
EfficiencyFactory& EfficiencyFactory::SetMomBins(double t_min, double t_max, int t_bins) 
{ mom_bin_ = {t_min, t_max, t_bins}; return *this;};
EfficiencyFactory& EfficiencyFactory::SetThetaBins(double t_min, double t_max, int t_bins) 
{ theta_bin_ = {t_min, t_max, t_bins}; return *this;};
EfficiencyFactory& EfficiencyFactory::SetPhiBins(double t_min, double t_max, int t_bins) 
{ phi_bin_ = {t_min, t_max, t_bins}; return *this;};
EfficiencyFactory& EfficiencyFactory::OpenDB(const std::string& t_efficiency_db)
{ data_filename_ = t_efficiency_db; return *this;}
EfficiencyFactory& EfficiencyFactory::SetPhiCut(const std::vector<std::pair<double, double>>& t_phi_cut)
{ phi_cut_ = t_phi_cut; return *this;}
EfficiencyFactory& EfficiencyFactory::SetPhiRange(const std::vector<std::pair<double, double>>& t_phi_range)
{ phi_range_ = t_phi_range; return *this;}


TEfficiency EfficiencyFactory::FinalizeBins(bool t_verbose, 
                                            bool t_enable_phi, 
                                            bool t_use_va, 
                                            bool t_use_eff_mult, 
                                            double t_min_mult_dist_2_vert)
{
  TChain *tree = new TChain("cbmsim");
  tree -> Add(data_filename_.c_str());

  TH2F *det = new TH2F("det", "det", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  TH2F *truth = new TH2F("truth", "truth", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  
  TString condition = "STData[0].vaEmbedTag && PDG.fElements == 1000010030 && ";
  auto copied_cut = phi_cut_;
  int i = 0;
  if(copied_cut[i].first > 180) copied_cut[i].first -= 360.;
  if(copied_cut[i].second > 180) copied_cut[i].second -= 360.;
  if(copied_cut[i].first > copied_cut[i].second) { auto temp = copied_cut[i].first; copied_cut[i].first = copied_cut[i].second; copied_cut[i].first = temp; }

  condition += TString::Format("((STData[0].vaMom.Phi()*TMath::RadToDeg() > %f && %f > STData[0].vaMom.Phi()*TMath::RadToDeg()",
                               copied_cut[i].first, copied_cut[i].second);
  for(i = 1; i < copied_cut.size(); ++i)
  {  
    if(copied_cut[i].first > 180) copied_cut[i].first -= 360.;
    if(copied_cut[i].second > 180) copied_cut[i].second -= 360.;
    if(copied_cut[i].first > copied_cut[i].second) { auto temp = copied_cut[i].first; copied_cut[i].first = copied_cut[i].second; copied_cut[i].first = temp; }
    condition += TString::Format(") || (STData[0].vaMom.Phi()*TMath::RadToDeg() > %f && %f > STData[0].vaMom.Phi()*TMath::RadToDeg()",
                                 copied_cut[i].first, copied_cut[i].second);
  }
  condition += "))";

  condition += TString::Format(" && STData[0].vaNRowClusters + STData[0].vaNLayerClusters > %d && STData[0].recodpoca.Mag() < %f",
                               num_clusters_, dist_2_vert_);

  std::cout << condition << std::endl;
  tree -> Project("det", "STData[0].embedMom.Theta()*TMath::RadToDeg():STData[0].embedMom.Mag()", condition);
  tree -> Project("truth", "STData[0].embedMom.Theta()*TMath::RadToDeg():STData[0].embedMom.Mag()");
 
  TEfficiency efficiency(*det, *truth);
  efficiency.SetDirectory(0);
  return efficiency;
};

int EfficiencyFactory::_EffectiveMult(const std::vector<double>& t_AllDistToVertex, double t_dist_2_vert)//const std::vector<int>& t_AllClusterNum, double t_dist_2_vert, double t_num_clusters)
{
  int count = 0;
  int size = t_AllDistToVertex.size();
  for(int i = 0; i < size; ++i)
    if(t_AllDistToVertex[i] <= t_dist_2_vert) ++count;
  return count;
};

bool EfficiencyFactory::_InsidePhi(double t_phi, const std::vector<std::pair<double, double>>& t_phi_list)
{
  // if it is too slow, then we need to implement interval tree
  for(const auto& phi_interval: t_phi_list)
    if(phi_interval.first < t_phi && t_phi <= phi_interval.second) return true;
  return false;
};
