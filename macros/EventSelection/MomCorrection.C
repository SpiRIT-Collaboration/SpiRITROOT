#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/EventSelection/ChooseFolder.h"
#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/ProtonCutGeneration/AutoProtonCut.cxx"

void MomCorrection(bool t_use_va, int t_sd, bool t_make_cut=true, bool inspect = false, bool pion = false)
{
  double mom_low = 350;
  double mom_high = 400;

  auto files = AggrateFolder(mom_low, mom_high, 0, 60, 0, 0, false, "", -1, (pion)? "data/Run2841_WithPions/" : "data/Run2841_WithProton/TrackDistComp/");
  TChain chain("cbmsim");
  for(auto name : files) 
  {
    name += "/run2841_s*";
    std::cout << "Loading file: " << name << std::endl;
    chain.Add(name.c_str());
  }

  std::vector<double> theta_cut{0, 5, 10, 15, 20, 25, 30, 35, 40, 45, 50, 55, 60};
  std::vector<double> phi_cut{-180, -150, -120, -90, -60, -30, 0, 30, 60, 90, 120, 150, 180};
  std::string name = "cut" + std::to_string(t_sd) + "sd";
  if(t_use_va) name += "VAtrack";
  if(pion) name = "pion_" + name;

  auto cut_name = TString::Format("Cut_%dSD.root", t_sd);
  TFile pion_cut("piCuts.root");
  TCutG* cutg = nullptr;
  if(pion) 
  {
    cut_name = "piCuts.root";
    cutg = (TCutG*) pion_cut.Get("pip");
  }
  else
  {
    if(t_use_va) cut_name = "VA" + cut_name;
    cutg = AutoProtonCut(t_sd, t_use_va);
    if(t_make_cut) cutg->SaveAs(cut_name.Data());
  }
  

  RuleTree tree;
  tree.AppendRule<MomentumTracks>("Mom", 3);
  tree.AppendRule<ValueCut>("MomCut", mom_low, mom_high);
  tree.AppendRule<ThetaPhi>("angular");
  tree.AppendSwitch("theta_switch", theta_cut);
  tree.AppendSwitch("phi_switch", phi_cut, true);
  tree.AppendRule<EmbedFilter>("EFilter");
  tree.AppendRule<RecoTrackClusterNumFilter>("CFilter", [](int t_tracks){return t_tracks > 20;});
  
  tree.AppendRule<DistToVertex>("avert");
  tree.AppendRule<ValueCut>("acut", 20);//, (i==0)? 5 : ((i==1)? 10 : 20));
  if(t_use_va) tree.AppendRule<UseVATracks>("vatrack");
  tree.AppendRule<PID>("pid");
  tree.AppendRule<EmbedCut>("pid_cut", cut_name.Data(), (pion)? "pip":"CUTG");
  tree.AppendRule<CompareMCPrimary>("comparemc", CompareMCPrimary::MMagDiff, CompareMCPrimary::StartMagDiff);

  auto angular = tree.Inspect("EFilter", "EmbedData", "Embeded Angular distribution;#theta (deg);#phi (deg)"
                              ,20,0,40,25,-180,180);
  auto pid = tree.Inspect("pid", "PID_Embed", "PID of the embedded protons;Momentum (MeV/c);dE/dx (arb. unit)"
                          ,100,0.8*mom_low,1.2*mom_high,1000, 0,1000);
  auto compare_mc = tree.Inspect("comparemc", "RejectedMomRes_EmbedData", "Momentum of the rejected particles;Momentum (MeV/c2);Dist to vertex (mm)"
                                 ,50,-50,50,500,-20,20);
  chain.LoadBaskets(2000000000);
  tree.DrawMultiple(&chain);

  /*************************************
  // save the results after calculation
  **************************************/

  {
    DailyLogger logger(TString::Format("Angular_mom_%g_%g", mom_low, mom_high).Data(), name + ".root");
    for(int i = 0; i < theta_cut.size()-1; ++i)
      for(int j = 0; j < phi_cut.size()-1; ++j)
      {
        //auto hist = GetMCDist(chain,cut[j-1], cut[j], mom_low, mom_high);
        angular[i*(phi_cut.size()-1)+j]->SetName(TString::Format("mc_angular_theta_%g_%g_phi_%g_%g", theta_cut[i], theta_cut[i+1], phi_cut[j], phi_cut[j+1]).Data());
        pid[i*(phi_cut.size()-1)+j]->SetName(TString::Format("pid_theta_%g_%g_phi_%g_%g", theta_cut[i], theta_cut[i+1], phi_cut[j], phi_cut[j+1]).Data());
        compare_mc[i*(phi_cut.size()-1)+j]->SetName(TString::Format("comparemc_theta_%g_%g_phi_%g_%g", theta_cut[i], theta_cut[i+1], phi_cut[j], phi_cut[j+1]).Data());
        logger.SaveClass(*angular[i*(phi_cut.size()-1)+j]);
        logger.SaveClass(*pid[i*(phi_cut.size()-1)+j]);
        logger.SaveClass(*compare_mc[i*(phi_cut.size()-1)+j]);
      }

    TGraph2D correction;
    for(int i = 0; i < theta_cut.size()-1; ++i)
      for(int j = 0; j < phi_cut.size()-1; ++j)
      {
        double phi_ave = 0.5*(phi_cut[j] + phi_cut[j+1]);
        double theta_ave = 0.5*(theta_cut[i] + theta_cut[i+1]);
        TF1 gaus("tf1", "gaus");
        auto proj = compare_mc[i*(phi_cut.size()-1)+j]->ProjectionX();
        proj->Fit(&gaus);
        if(proj->GetEntries() > 0) correction.SetPoint(correction.GetN(), theta_ave, phi_ave, gaus.GetParameter(1));
      }
    correction.SetName(TString::Format("Mom_corr").Data());
    logger.SaveClass(correction);
  }
}

void MomCorrection()
{
  for(int use_va = 0; use_va < 2; ++use_va)
    for(int sd = 1; sd < 4; ++sd)
      MomCorrection((use_va == 1), sd, true, false, false);
}
