#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/EventSelection/ChooseFolder.h"

void VertexVsPhi()
{
  RuleTree tree;
  tree.AppendRule<ThetaPhi>("angle");
  tree.AppendRule<EmbedFilter>("EFilter");
  tree.AppendRule<DistToVertex>("dist");
  tree.AppendRule<Last2Rules2XY>("XY");
  tree.WireTap("XY", 0);

  auto files = AggrateFolder(350, 450, 0, 30, 0, 0, false, "/run2841_s*.reco.tommy_branch.1754.b185c90.root");
  TChain chain("cbmsim");
  for(const auto& name : files)
  {
    std::cout << "Loading file: " << name << std::endl;
    chain.Add(name.c_str());
  }

  DrawMultipleComplex drawer(&chain);
  TH2F hist("hist", "hist", 180, 0, 180, 200, 0, 50);
  drawer.DrawMultiple(*tree.first_rule_.get(), hist);

  DailyLogger logger("VertexVsPhi");
  logger.SaveClass(hist);
}
