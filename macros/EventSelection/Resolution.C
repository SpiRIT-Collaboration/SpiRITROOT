#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/EventSelection/ChooseFolder.h"


void Resolution(const std::string& t_cutname="NewProtonCut.root")
{
  RuleTree tree;
  tree.AppendRule<RecoTrackClusterNumFilter>("CFilter", [](int t_clusters){ return t_clusters>20;});
  tree.AppendRule<EmbedFilter>("EFFilter");
  tree.AppendRule<PID>("pid");
  tree.AppendRule<EmbedCut>("pid_cut", t_cutname.c_str());
  tree.AppendRule<MomentumTracks>("Mom", 3);
  tree.AppendRule<ValueCut>("MomCut", 350, 400);
  tree.AppendRule<ThetaPhi>("thetaphi");

  std::vector<TH1F> resolution;
  std::vector<double> theta_bound{0, 10, 20, 30};
  std::vector<double> phi_bound{-180,-150,-120,-90,-60,-30,0.,30.,60.,90.,120,150,180};

  tree.AppendXYSwitch("PIDSwitch", theta_bound, phi_bound);
  tree.AppendRuleToSwitch<CompareMCPrimary>("PIDSwitch", "comp", CompareMCPrimary::MMagDiff, CompareMCPrimary::None);

  int index = 0;
  for(int j = 0; j < phi_bound.size()-1; ++j)
    for(int i = 0; i < theta_bound.size()-1; ++i)
    {
      tree.WireTap("comp_" + std::to_string(index), index);
      resolution.emplace_back(TString::Format("Theta_%.1f_%.1f_Phi_%.1f_%.1f", 
                                              theta_bound[i], 
                                              theta_bound[i+1],
                                              phi_bound[j],
                                              phi_bound[j+1]).Data(), "", 1000, -500, 500);
      ++index;
    }

  auto files = AggrateFolder(350, 450, 0, 30, 0, 0, false, "/run2841_s*.reco.tommy_branch.1753.688ee41.root");
  TChain chain("cbmsim");
  for(const auto& name : files)
  {
    std::cout << "Loading file: " << name << std::endl;
    chain.Add(name.c_str());
  }


  DrawMultipleComplex drawer(&chain);//"data/Run2841_WithProton/TrackDistComp/Mom_350.0_450.0_Theta_0.0_10.0_Group_0/run2841_s*", "cbmsim");
  drawer.DrawMultiple(*tree.first_rule_.get(), resolution);

  DailyLogger logger("Resolution");
  for(const auto& hist : resolution)
    logger.SaveClass(hist);
  
}
