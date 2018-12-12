#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/EventSelection/ChooseFolder.h"

void ComparePID()
{
  RuleTree tree;
  tree.AppendRule<RecoTrackClusterNumFilter>("CFilter", [](int t_tracks){return t_tracks > 20;});
  tree.AppendRule<ThetaPhi>("angular");
  tree.AppendRule<ValueCut>("90deg", 80, 110, true);
  tree.AppendRule<EmbedFilter>("EFilter90");
  tree.AppendRule<PID>("pidembed");

  tree.AppendRejectedRuleTo<PID>("EFilter90", "pidreal");

  tree.AppendRejectedRuleTo<ValueCut>("90deg", "-90deg", -110, -80, true);
  tree.AppendRejectedRuleTo<EmbedFilter>("90deg", "EFilter-90");
  tree.AppendRejectedRuleTo<PID>("90deg", "pidnegembed");

  tree.AppendRejectedRuleTo<PID>("EFilter-90", "pidnegreal");
  

  tree.WireTap("pidembed", 0);
  tree.WireTap("pidreal", 1);
  tree.WireTap("pidnegembed", 2);
  tree.WireTap("pidnegreal", 3);

  TH2F embedpos("embedpos", "embedpos", 1000, 0, 1000, 1000, 0, 1000);
  TH2F realpos("realpos", "realpos", 1000, 0, 1000, 1000, 0, 1000);
  TH2F embedneg("embedneg", "embedneg", 1000, 0, 1000, 1000, 0, 1000);
  TH2F realneg("realneg", "realneg", 1000, 0, 1000, 1000, 0, 1000);

  auto files = AggrateFolder(350, 450, 0, 30, 0, 0, false, "/run2841_s*.reco.tommy_branch.1753.688ee41.root");
  TChain chain("cbmsim");
  for(const auto& name : files) 
  {
    std::cout << "Loading file: " << name << std::endl;
    chain.Add(name.c_str());
  }
            
  DrawMultipleComplex drawer(&chain);
  drawer.DrawMultiple(*tree.first_rule_.get(), embedpos, realpos, embedneg, realneg);

  DailyLogger logger("PID_test");
  logger.SaveClass(embedpos);
  logger.SaveClass(realpos);
  logger.SaveClass(embedneg);
  logger.SaveClass(realneg);
}
