#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/EventSelection/ChooseFolder.h"

void NewEfficiency(TH2F& t_raw_mc_350_400, TH2F& t_raw_mc_400_450, RuleTree& t_additional_tree, const std::string& t_name="", const std::string& t_cutname = "NewProtonCut.root")
{
  RuleTree tree;
  tree.AppendRule<RecoTrackClusterNumFilter>("CFilter", [](int t_tracks){return t_tracks > 20;});
  tree.AppendRule<PID>("pid");
  tree.AppendRule<EmbedCut>("pid_cut", t_cutname);
  tree.AppendTree(t_additional_tree);
  tree.AppendRule<MomentumTracks>("Mom", 3);
  tree.AppendSwitch("MomSwitch", {350,400,450});
  tree.AppendRuleToSwitch<EmbedFilter>("MomSwitch", "EFilter");
  tree.AppendRuleToSwitch<ThetaPhi>("MomSwitch", "angular");

  tree.AppendRejectedRuleToAll<ThetaPhi>("EFilter", "angular_real");

  tree.WireTap("angular_0", 0);
  tree.WireTap("angular_real", 1);
  tree.WireTap("angular_1", 2);
  tree.WireTap("angular_real_1", 3);

  auto files = AggrateFolder(350, 450, 0, 30, 0, 0, false, "/run2841_s*.reco.tommy_branch.1753.688ee41.root");
  TChain chain("cbmsim");
  for(const auto& name : files) 
  {
    std::cout << "Loading file: " << name << std::endl;
    chain.Add(name.c_str());
  }
            
  DrawMultipleComplex drawer(&chain);
  TH2F hist("EmbedData_350_400", "Embeded Angular distribution;#theta (deg);#phi (deg)", 40,0,40,50,-180,180);
  TH2F hist_real("RealData_350_400", "Real Angular distribution;#theta (deg);#phi (deg)", 40,0,40,50,-180,180);
  TH2F hist_400("EmbedData_400_450", "Embeded Angular distribution;#theta (deg);#phi (deg)", 40,0,40,50,-180,180);
  TH2F hist_real_400("RealData_400_450", "Real Angular distribution;#theta (deg);#phi (deg)", 40,0,40,50,-180,180);
  drawer.DrawMultiple(*tree.first_rule_.get(), hist, hist_real, hist_400, hist_real_400);

  DailyLogger logger(t_name);
  logger.SaveClass(hist);
  logger.SaveClass(hist_real);
  logger.SaveClass(hist_400);
  logger.SaveClass(hist_real_400);
  // calculate Efficiency
  // rebin x y to make it smoother
  hist.RebinX(2);
  hist.RebinY(2);
  hist_real.RebinX(2);
  hist_real.RebinY(2);
  hist_400.RebinX(2);
  hist_400.RebinY(2);
  hist_real_400.RebinX(2);
  hist_real_400.RebinY(2);

  if(hist.GetNbinsX() != t_raw_mc_350_400.GetNbinsX())
  {
      t_raw_mc_350_400.RebinX(2);
      t_raw_mc_350_400.RebinY(2);
  }
  if(hist.GetNbinsX() != t_raw_mc_400_450.GetNbinsX())
  {
      t_raw_mc_400_450.RebinX(2);
      t_raw_mc_400_450.RebinY(2);
  }

  hist.Divide(&t_raw_mc_350_400);
  hist.SetName("efficiency_350_400");
  logger.SaveClass(hist);
  
  hist_real.Divide(&hist);
  hist_real.SetName("corrected_dist_350_400");
  logger.SaveClass(hist_real);

  hist_400.Divide(&t_raw_mc_400_450);
  hist_400.SetName("efficiency_400_450");
  logger.SaveClass(hist_400);
  
  hist_real_400.Divide(&hist);
  hist_real_400.SetName("corrected_dist_400_450");
  logger.SaveClass(hist_real_400);
  
}

TH2F CollectMCData(double t_low=350, double t_high=400)
{
  // find real mc numbers
  auto files = AggrateFolder(350, 450, 0, 30, 0, 0, false, ".mc.root");
  TChain chain("cbmsim");
  for(const auto& name : files) chain.Add(name.c_str());
  TH2F mc_hist("mc_num", "mc_num;#theta (deg);#phi (deg)", 40, 0, 40, 50, -180, 180);

  const std::string var("TMath::ATan2(PrimaryTrack.fPy, PrimaryTrack.fPx)*180/3.1415926");
  const std::string var2("atan(sqrt(PrimaryTrack.fPx*PrimaryTrack.fPx + PrimaryTrack.fPy*PrimaryTrack.fPy)/PrimaryTrack.fPz)*180/3.1415926");
  const std::string mag("sqrt(PrimaryTrack.fPx*PrimaryTrack.fPx + PrimaryTrack.fPy*PrimaryTrack.fPy + PrimaryTrack.fPz*PrimaryTrack.fPz)");
  chain.Draw((var + ":" + var2 + ">>mc_num").c_str(), TString::Format("%f < %s && %s < %f", t_low/1000., mag.c_str(), mag.c_str(), t_high/1000.).Data(), "goff");

  DailyLogger logger(TString::Format("MCProtons_%.1f_%.1f", t_low, t_high).Data());
  logger.SaveClass(mc_hist);
  return mc_hist;
}

void NewEfficiency()
{
  auto mc_hist_350_400 = CollectMCData(350, 400);
  auto mc_hist_400_450 = CollectMCData(400, 450);
  {
    RuleTree additional;
    additional.AppendRule<DistToVertex>("avert");
    additional.AppendRule<ValueCut>("acut", 0, 10);
    NewEfficiency(mc_hist_350_400, mc_hist_400_450, additional, "DistToVert10");
  }
  {
    RuleTree additional;
    additional.AppendRule<DistToVertex>("avert");
    additional.AppendRule<ValueCut>("acut", 0, 20);
    NewEfficiency(mc_hist_350_400, mc_hist_400_450, additional, "DistToVert20");
  }
  {
    RuleTree additional;
    additional.AppendRule<DistToVertex>("avert");
    additional.AppendRule<ValueCut>("acut", 0, 4);
    NewEfficiency(mc_hist_350_400, mc_hist_400_450, additional, "DistToVert5");
  }
  /*{
    RuleTree additional;
    additional.AppendRule<DistToVertex>("avert");
    additional.AppendRule<ValueCut>("acut", 0, 10);
    additional.AppendRule<UseVATracks>("va");
    NewEfficiency(mc_hist_350_400, mc_hist_400_450, additional, "VADistToVert10");
  }
  {
    RuleTree additional;
    additional.AppendRule<DistToVertex>("avert");
    additional.AppendRule<ValueCut>("acut", 0, 20);
    additional.AppendRule<UseVATracks>("va");
    NewEfficiency(mc_hist_350_400, mc_hist_400_450, additional, "VADistToVert20");
  }*/

}
