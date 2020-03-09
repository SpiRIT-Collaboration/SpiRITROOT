#include "TH2.h"
#include "TChain.h"
#include "EfficiencyFactory.hh"
#include "TDatabasePDG.h"
#include <glob.h>

std::vector<std::string> glob(const char *pattern) {
    glob_t g;
    glob(pattern, GLOB_TILDE, nullptr, &g); // one should ensure glob returns 0!
    std::vector<std::string> filelist;
    filelist.reserve(g.gl_pathc);
    for (size_t i = 0; i < g.gl_pathc; ++i) {
        filelist.emplace_back(g.gl_pathv[i]);
    }
    globfree(&g);
    return filelist;
}

   
EfficiencyFactory& EfficiencyFactory::SetMomBins(double t_min, double t_max, int t_bins) 
{ mom_bin_ = {t_min, t_max, t_bins}; return *this;};
EfficiencyFactory& EfficiencyFactory::SetThetaBins(double t_min, double t_max, int t_bins) 
{ theta_bin_ = {t_min, t_max, t_bins}; return *this;};
EfficiencyFactory& EfficiencyFactory::SetPhiCut(const std::vector<std::pair<double, double>>& t_phi_cut)
{ phi_cut_ = t_phi_cut; return *this;}
EfficiencyFactory& EfficiencyFactory::SetTrackQuality(int t_nclus, double t_poca)
{ dist_2_vert_ = t_poca; num_clusters_ = t_nclus; return *this;}


/*********************************************
*  OrigEfficiency
**********************************************/
OrigEfficiencyFactory::OrigEfficiencyFactory(const std::string& t_efficiency_db) : data_filename_(t_efficiency_db)
{}
TEfficiency OrigEfficiencyFactory::FinalizeBins(int t_pdg,
                                            bool t_verbose)
{
  TFile file(data_filename_.c_str());
  if(!file.IsOpen()) throw std::runtime_error("EfficiencyFactory: File " + data_filename_ + " not found.");

  std::string treename;
  switch(t_pdg)
  {  
    case 211: treename = "pi+"; break;
    case -211: treename = "pi-"; break;
    case 2212: treename = "proton"; break;
    case 1000010020: treename = "deuteron"; break;
    case 1000010030: treename = "triton"; break;
    case 1000020030: treename = "he3"; break;
    default:
      throw std::runtime_error("PDG " + std::to_string(t_pdg) + " is not yet supported");
  }

  auto tree = (TTree*) file.Get(treename.c_str());
  if(!tree) throw std::runtime_error("EfficiencyFactory: TTree \"" + treename + "\" not found in " + data_filename_);

  TTreeReader reader(tree);
  typedef TTreeReaderValue<double> dvalue;
  typedef TTreeReaderValue<int> ivalue;
  typedef TTreeReaderValue<bool> bvalue;

  std::string prefix = "VA";

  dvalue P(reader, "P"), Theta(reader, "Theta"), Phi(reader, "Phi");
  dvalue P_reco(reader, (prefix + "P_reco").c_str()); 
  dvalue Theta_reco(reader, (prefix + "Theta_reco").c_str()); 
  dvalue Phi_reco(reader, (prefix + "Phi_reco").c_str());
  dvalue dist_to_vertex(reader, "DistToVertex"), dEdX(reader, "DeDx");
  ivalue clusters(reader, "ClusterNum"), charge(reader, "Charge"), mult(reader, "mult");
  bvalue detected(reader, "Detected");

  TEfficiency efficiency("tefficiency", 
                         "tefficiency",
                         mom_bin_.nbins, 
                         mom_bin_.min, 
                         mom_bin_.max,
                         theta_bin_.nbins, 
                         theta_bin_.min, 
                         theta_bin_.max);

  while(reader.Next())
  {
    if(t_verbose) 
    {
      std::cout << "EfficiencyFactory progress: " << std::setw(3) << std::setfill(' ') << int(100*(reader.GetCurrentEntry()+1)/(double)reader.GetEntries(true)) << " %  \r";
      if(reader.GetEntries(true) == reader.GetCurrentEntry()+1) std::cout << std::flush;
    }
      

    // check cut conditions
    bool satisfy_cuts = false;
    bool insidePhi = false;
    for(const auto& phiCut : phi_cut_) 
      if(phiCut.first < *Phi_reco && *Phi_reco < phiCut.second)
      {
        insidePhi = true;
        break;
      }
    if(insidePhi && *clusters > num_clusters_
       && *dist_to_vertex < dist_2_vert_
       && *detected)  
    {
      satisfy_cuts = true;
    }else satisfy_cuts = false;
    efficiency.Fill(satisfy_cuts, *P, *Theta);
  }
  efficiency.SetDirectory(0);
  return efficiency;
}

/********************************************************
* Efficiency from ana 
*********************************************************/

EfficiencyFromAnaFactory::EfficiencyFromAnaFactory(){}

void EfficiencyFromAnaFactory::SetDataBaseForPDG(int t_pdg, const std::string& t_efficiency_db)
{
  fEfficiencyDB[t_pdg] = t_efficiency_db;
}

TEfficiency EfficiencyFromAnaFactory::FinalizeBins(int t_pdg,
                                                   bool t_verbose)
{
  TChain tree("cbmsim");
  for(const auto& filename : glob(fEfficiencyDB[t_pdg].c_str()))
    tree.Add(filename.c_str());

  TH2F *det = new TH2F("det", "det", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  TH2F *truth = new TH2F("truth", "truth", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  
  TString condition = TString::Format("STData[0].vaEmbedTag && PDG.fElements == %d && ", t_pdg);
  TString wrappedPhi = "((STData[0].vaMom.Phi() < 0)? STData[0].vaMom.Phi()*TMath::RadToDeg() + 360:STData[0].vaMom.Phi()*TMath::RadToDeg())";
  condition += "(";
  for(int i = 0; i < phi_cut_.size(); ++i)
  {  
    if(i > 0) condition += ") || ";
    condition += TString::Format("(%s > %f && %f > %s",
                                 wrappedPhi.Data(), phi_cut_[i].first, phi_cut_[i].second, wrappedPhi.Data());
  }
  condition += "))";

  condition += TString::Format(" && STData[0].vaNRowClusters + STData[0].vaNLayerClusters > %d && STData[0].recodpoca.Mag() < %f",
                               num_clusters_, dist_2_vert_);

  tree.Project("det", "STData[0].embedMom.Theta()*TMath::RadToDeg():STData[0].embedMom.Mag()", condition);
  tree.Project("truth", "STData[0].embedMom.Theta()*TMath::RadToDeg():STData[0].embedMom.Mag()");
 
  TEfficiency efficiency(*det, *truth);
  det -> SetDirectory(0);
  truth -> SetDirectory(0);
  efficiency.SetDirectory(0);
  return efficiency;
};

/********************************************************
* Efficiency from conc
*********************************************************/

EfficiencyFromConcFactory::EfficiencyFromConcFactory(){}

void EfficiencyFromConcFactory::SetDataBaseForPDG(int t_pdg, const std::string& t_efficiency_db)
{
  fEfficiencyDB[t_pdg] = t_efficiency_db;
}

TEfficiency EfficiencyFromConcFactory::FinalizeBins(int t_pdg,
                                                    bool t_verbose)
{
  TChain tree("spirit");
  for(const auto& filename : glob(fEfficiencyDB[t_pdg].c_str()))
    tree.Add(filename.c_str());

  TH2F *det = new TH2F("det", "det", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  TH2F *truth = new TH2F("truth", "truth", mom_bin_.nbins, mom_bin_.min, mom_bin_.max, theta_bin_.nbins, theta_bin_.min, theta_bin_.max);
  
  if(tree.GetEntries() > 0)
  {
    TString condition = TString::Format("vaEmbedTag && ");
    //TString wrappedPhi = "((vaMom.Phi() < 0)? vaMom.Phi()*TMath::RadToDeg() + 360:vaMom.Phi()*TMath::RadToDeg())";
    TString wrappedPhi = "((embedMom.Phi() < 0)? embedMom.Phi()*TMath::RadToDeg() + 360:embedMom.Phi()*TMath::RadToDeg())";
    condition += "(";
    for(int i = 0; i < phi_cut_.size(); ++i)
    {  
      if(i > 0) condition += ") || ";
      condition += TString::Format("(%s > %f && %f > %s",
                                   wrappedPhi.Data(), phi_cut_[i].first, phi_cut_[i].second, wrappedPhi.Data());
    }
    condition += "))";

    condition += TString::Format(" && vaNRowClusters + vaNLayerClusters > %d && recodpoca.Mag() < %f",
                                 num_clusters_, dist_2_vert_);

    tree.Project("det", "embedMom.Theta()*TMath::RadToDeg():embedMom.Mag()", condition);
    tree.Project("truth", "embedMom.Theta()*TMath::RadToDeg():embedMom.Mag()");

    det -> Smooth();
    truth -> Smooth();
  } else  
    std::cout << "No data is loaded for particle " << TDatabasePDG::Instance()->GetParticle(t_pdg)->GetName() << ". Will return an empty efficiency instead." << std::endl; 
 
  TEfficiency efficiency(*det, *truth);
  det -> SetDirectory(0);
  truth -> SetDirectory(0);
  efficiency.SetDirectory(0);
  return efficiency;
};
