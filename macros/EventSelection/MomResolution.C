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

void MomResolution(double mom_min, double mom_max, double phi_min, double phi_max, double theta_min, double theta_max)
{
  TH1::AddDirectory(false);
  auto filename_list = AggrateFolder(mom_min, mom_max, theta_min, theta_max);
  auto mc_list = AggrateFolder(mom_min, mom_max, theta_min, theta_max, ".mc.root");

  TChain mc_chain("cbmsim");
  for(const auto& name : mc_list)
    mc_chain.Add(name.c_str());
  GetMomentumListFromMC(&mc_chain);

  TrackZFilter min_z_filter;
  EmbedFilter embed_filter;
  PID pid;
  EmbedCut pid_cut("ProtonCut.root");
  ThetaPhi theta_phi;
  ValueCut phi_cut(phi_min, phi_max, true), theta_cut(theta_min, theta_max);
  /*MomentumTracks mom(3);
  ValueCut mom_cut(mom_min, mom_max);*/
  CompareMCPrimary compare("MomDist.txt", CompareMCPrimary::MomZ, CompareMCPrimary::None);

  TChain chain("cbmsim");
  std::cout << "Loading data from:\n";
  for(const auto& name : filename_list)
  {
    std::cout << name << std::endl;
    chain.Add(name.c_str());
  }

  DrawMultipleComplex drawer(&chain);
  auto cp = ListOfCP(2);

  min_z_filter.AddRule(embed_filter.AddRule(
                       pid.AddRule(
                       pid_cut.AddRule(
                       theta_phi.AddRule(
                       phi_cut.AddRule(
                       theta_cut.AddRule(
                       /*mom.AddRule(
                       mom_cut.AddRule(*/
                       cp[1].AddRule(
                       compare.AddRule(&cp[0])))))))))/*))*/;

  TH1F mom_resolution("mom_res", "Momentum resolution;Real momentum - Reco momentum;", 200, -100, 100);
  TH1F mom_distribution("mom_dist", "Momentum distribution;Reco momentum;", 300, 200, 500);
  drawer.DrawMultiple(min_z_filter, mom_resolution, mom_distribution);
  TF1 gaus("gaus", "gaus", -100, 100);
  mom_resolution.Fit(&gaus);

  DailyLogger logger(std::string(TString::Format("Mom_%.1f_%.1f_Theta_%.1f_%.1f_Phi_%.1f_%.1f", mom_min, mom_max, theta_min, theta_max, phi_min, phi_max).Data()));
  logger.SaveClass(mom_resolution);
  logger.SaveClass(mom_distribution);
  logger.SaveClass(gaus);
}

void MomResolutionAll()
{
  std::vector<double> phi{-30,0,30,60,90,120,160,180,210};
  std::vector<double> mom{350,450};
  std::vector<double> theta{0,5,10,20,30,40};
  for(int k = 0; k < phi.size()-1; ++k)
    for(int i = 0; i < mom.size()-1; ++i)
      for(int j = 0; j < theta.size()-1; ++j)
        MomResolution(mom[i], mom[i+1], phi[k], phi[k+1], theta[j], theta[j+1]);
}
