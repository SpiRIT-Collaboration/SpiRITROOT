#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/EventSelection/ChooseFolder.h"

void GetMomentumListFromMC(TChain *tree, const std::string& t_output_name = "MomDist.txt")
{
  std::ofstream output(t_output_name.c_str());
  if(!output.is_open()) std::cerr << "Cannot open output file \n";
  output << "Entry\tPx\tPy\tPz\tStartX\tStartY\tStartZ\n";

  TTreeReader reader(tree);
  TTreeReaderValue<TClonesArray> Momentum(reader, "PrimaryTrack");

  while(reader.Next())
  {
    auto MomArray = *Momentum;
    for(unsigned i = 0; i < MomArray.GetEntries(); ++i)
    {
      auto ptrack = (STMCTrack*) MomArray.At(i);
      output << reader.GetCurrentEntry() << "\t" << ptrack->GetPx() << "\t" << ptrack->GetPy() << "\t" << ptrack->GetPz() << "\t"
             << ptrack->GetStartX() << "\t" << ptrack->GetStartY() << "\t" << ptrack->GetStartZ() << "\n";   
    }
  }
}

void GetMomentumListFromMC(const std::string& t_filename = "data/Run2841_WithProton/TrackDistComp/Mom_350.0_400.0_Theta_0.0_10.0.mc.root", const std::string& t_output_name = "MomDist.txt")
{
  TFile file(t_filename.c_str());
  if(!file.IsOpen()) std::cerr << "File cannot be opened\n";
  TTree *tree = (TTree*) file.Get("cbmsim");
  if(!tree) std::cerr << "Tree cbmsim is not found in the file\n";

  GetMomentumListFromMC((TChain*) tree, t_output_name);
}

void MomResolutionSwitch(double mom_min, double mom_max, double theta_min, double theta_max, TH2F& t_all)
{
  std::vector<std::pair<double, double>> phi_bounds;
  for(double angle = -190; angle < 190; angle += 20)
    phi_bounds.push_back({angle, angle+20});


  TH1::AddDirectory(false);
  auto filename_list = AggrateFolder(mom_min, mom_max, theta_min, theta_max, "_WithBDC/run2841_s*.reco.tommy_branch.1750.b9d095a.root");
  /*auto mc_list = AggrateFolder(mom_min, mom_max, theta_min, theta_max, ".mc.root");

  TChain mc_chain("cbmsim");
  for(const auto& name : mc_list)
    mc_chain.Add(name.c_str());
  GetMomentumListFromMC(&mc_chain);*/

  // load chains
  TChain chain("cbmsim");
  std::cout << "Loading data from:\n";
  for(const auto& name : filename_list)
  {
    std::cout << name << std::endl;
    chain.Add(name.c_str());
  }

  DrawMultipleComplex drawer(&chain);
  auto cp = ListOfCP(3*phi_bounds.size() + 1);
  // create rules to fill
  TrackZFilter min_z_filter;
  EmbedFilter embed_filter;
  UseVATracks va_tracks;
  PID pid;
  EmbedCut pid_cut("ProtonCut.root");
  MCThetaPhi theta_phi;
  SwitchCut phi_cut(phi_bounds, true);
  std::vector<ThetaPhi> reco_theta_phi;
  std::vector<MomentumTracks> mom;
  std::vector<ValueCut> theta_cut;
  std::vector<CompareMCPrimary> compare;
  for(unsigned i = 0; i < phi_bounds.size(); ++i)
  {
    reco_theta_phi.push_back(ThetaPhi());
    mom.push_back(MomentumTracks(3));
    theta_cut.push_back(ValueCut(theta_min, theta_max));
    compare.push_back(CompareMCPrimary(/*"MomDist.txt", */CompareMCPrimary::MomZ, CompareMCPrimary::None));
  }

  // join the rules
  min_z_filter.AddRule(embed_filter.AddRule(
                       va_tracks.AddRule(
                       pid.AddRule(
                       pid_cut.AddRule(
                       theta_phi.AddRule(
                       cp[3*phi_bounds.size()].AddRule(
                       &phi_cut)))))));

  for(unsigned i = 0; i < phi_bounds.size(); ++i)
  {
    theta_cut[i].AddRule(reco_theta_phi[i].AddRule(
                         cp[3*i].AddRule(
                         mom[i].AddRule(
                         cp[3*i+1].AddRule(
                         compare[i].AddRule(&cp[3*i+2]))))));
    phi_cut.SwitchRule(i, &theta_cut[i]);
  }

  // fill histograms
  std::vector<TH2F> hists;  
  for(unsigned i = 0; i < phi_bounds.size(); ++i)
  {
    hists.push_back(TH2F("thetaphi", "#phi vs #theta;#theta;#phi", 40,0,40,180,-180,180));
    hists.push_back(TH2F("mom_dist", "Momentum distribution;Reco momentum;", 300, 200, 500, 10,0,2));
    hists.push_back(TH2F("mom_res", "Momentum resolution;Real momentum - Reco momentum;", 200, -100, 100, 10,0,2));
  }
  hists.push_back(TH2F("all_thetaphi", "#phi vs #theta for all statistics;#theta;#phi", 40,0,40,180,-180,180));

  drawer.DrawMultiple(min_z_filter, hists);

  for(unsigned i = 0; i < phi_bounds.size(); ++i)
  {
    DailyLogger logger(std::string(TString::Format("Mom_%.1f_%.1f_Theta_%.1f_%.1f_Phi_%.1f_%.1f_WithPID", mom_min, mom_max, theta_min, theta_max, phi_bounds[i].first, phi_bounds[i].second).Data()));
    logger.SaveClass(hists[3*i]);
    logger.SaveClass(hists[3*i+1]);
    logger.SaveClass(hists[3*i+2]);
  }
  t_all.Add(&hists.back());
}

void MomResolutionAll()
{
  std::vector<double> mom{350,450};
  std::vector<double> theta{0,5,10,15,20,25,30,35,40};
  TH2F all_dist("all_thetaphi", "#phi vs #theta for all statistics;#theta;#phi", 40,0,40,180,-180,180);
  for(int i = 0; i < mom.size()-1; ++i)
    for(int j = 0; j < theta.size()-1; ++j)
      MomResolutionSwitch(mom[i], mom[i+1], theta[j], theta[j+1], all_dist);

  DailyLogger logger("All_Ang_Dist");
  logger.SaveClass(all_dist);
}
