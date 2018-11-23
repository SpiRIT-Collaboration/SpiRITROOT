#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/EventSelection/ChooseFolder.h"

void EfficiencyCalculation(TH2F& t_raw_mc, Rule* t_additional_rules = 0, const std::string& t_name="", const std::string& t_cutname = "NewProtonCut.root")
{
    DailyLogger logger(t_name);

    TH2F real_dist, mc_dist;

    EmptyRule empty;
    if(!t_additional_rules) t_additional_rules = &empty;
    auto block = RuleBlock(t_additional_rules);

    for(const auto& type : std::vector<std::string>{"MC", "Real"})
    {
      auto checkpoints = ListOfCP(2);

      TrackZFilter min_track_num;
      RecoTrackClusterNumFilter clus_filter([](int t_tracks){return t_tracks > 20;});
      PID pid;
      EmbedCut pid_cut(t_cutname.c_str());
      EmbedFilter filter;
      //RecoTrackClusterNumFilter clus_filter([](int t_tracks){return t_tracks > 10;});
      ThetaPhi angular;
      ValueCut theta_cut(0, 30);
      MomentumTracks mom(3);
      ValueCut mom_cut(350, 450);
      DistToVertex vert;
            
      if(type == "MC")
        min_track_num.AddRule(clus_filter.AddRule(filter.AddRule(block.first)));
      else
        min_track_num.AddRule(clus_filter.AddRule(block.first));

      //clus_filter.AddRule(block.first);
      block.second->AddRule(pid.AddRule(pid_cut.AddRule(mom.AddRule(mom_cut.AddRule(angular.AddRule(theta_cut.AddRule(checkpoints[0].AddRule(vert.AddRule(&checkpoints[1])))))))));
      
      auto files = AggrateFolder(350, 450, 0, 30, 0, 0, false, "/run2841_s*.reco.tommy_branch.1752.ce5b587.root");
      TChain chain("cbmsim");
      for(const auto& name : files) 
      {
        std::cout << "Loading file: " << name << std::endl;
        chain.Add(name.c_str());
      }
              
      DrawMultipleComplex drawer(&chain);
      TH2F hist(type.c_str(), (type + ";#theta (deg);#phi (deg)").c_str(), 40,0,40,50,-180,180);
      TH1F hist_vert((type + "_vert").c_str(), "", 100, 0, 100);
      drawer.DrawMultiple(min_track_num, hist, hist_vert);

      /*hist.Draw("colz");
      gPad->WaitPrimitive();*/
      logger.SaveClass(hist);
      logger.SaveClass(hist_vert);
      if(type == "MC") 
        mc_dist = hist;
      else 
        real_dist = hist;
    }

    // calculate Efficiency
    // rebin x y to make it smoother
    mc_dist.RebinX(2);
    mc_dist.RebinY(2);
    real_dist.RebinX(2);
    real_dist.RebinY(2);

    if(real_dist.GetNbinsX() != t_raw_mc.GetNbinsX())
    {
        t_raw_mc.RebinX(2);
        t_raw_mc.RebinY(2);
    }

    mc_dist.Divide(&t_raw_mc);
    mc_dist.SetName("efficiency");
    logger.SaveClass(mc_dist);
   
    real_dist.Divide(&mc_dist);
    real_dist.SetName("corrected_dist");
    logger.SaveClass(real_dist);
}

TH2F CollectMCData()
{
  // find real mc numbers
  auto files = AggrateFolder(350, 450, 0, 30, 0, 0, false, ".mc.root");
  TChain chain("cbmsim");
  for(const auto& name : files) chain.Add(name.c_str());
  TH2F mc_hist("mc_num", "mc_num;#theta (deg);#phi (deg)", 40, 0, 40, 50, -180, 180);

  const std::string var("TMath::ATan2(PrimaryTrack.fPy, PrimaryTrack.fPx)*180/3.1415926");
  const std::string var2("atan(sqrt(PrimaryTrack.fPx*PrimaryTrack.fPx + PrimaryTrack.fPy*PrimaryTrack.fPy)/PrimaryTrack.fPz)*180/3.1415926");
  chain.Draw((var + ":" + var2 + ">>mc_num").c_str(), "", "goff");


  DailyLogger logger("MCProtons");
  logger.SaveClass(mc_hist);
  return mc_hist;
}
    

void EfficiencyTestCutConditions()
{
  auto mc_hist = CollectMCData();
  {
    DistToVertex vert;
    ValueCut cut(0, 20);
    UseVATracks vatrack;
    vert.AddRule(cut.AddRule(&vatrack));
    EfficiencyCalculation(mc_hist, &vert, "VADistToVert20");
  }
  {
    DistToVertex vert;
    ValueCut cut(0, 10);
    UseVATracks vatrack;
    vert.AddRule(cut.AddRule(&vatrack));
    EfficiencyCalculation(mc_hist, &vert, "VADistToVert10");
  }
  {
    DistToVertex vert;
    ValueCut cut(0, 20);
    vert.AddRule(&cut);
    EfficiencyCalculation(mc_hist, &vert, "DistToVert20");
  }
  {
    DistToVertex vert;
    ValueCut cut(0, 10);
    vert.AddRule(&cut);
    EfficiencyCalculation(mc_hist, &vert, "DistToVert10");
  }
  
  /*EfficiencyCalculation(mc_hist, 0, "WideProtonCut");
  {
    RecoTrackClusterNumFilter clus_filter([](int t_tracks){return t_tracks > 15;});
    EfficiencyCalculation(mc_hist, &clus_filter, "ClusFilter15");
  }

  {
    DistToVertex vert;
    ValueCut cut(0, 10);
    vert.AddRule(&cut);
    EfficiencyCalculation(mc_hist, &vert, "Tight_DistToVert10", "TightProtonCut.root");
  }
  EfficiencyCalculation(mc_hist, 0, "Tight", "TightProtonCut.root");
  {
    RecoTrackClusterNumFilter clus_filter([](int t_tracks){return t_tracks > 15;});
    EfficiencyCalculation(mc_hist, &clus_filter, "Tight_ClusFilter15");
  }

  {
    UseVATracks vatrack;
    DistToVertex vert;
    ValueCut cut(0, 10);
    vatrack.AddRule(vert.AddRule(&cut));
    EfficiencyCalculation(mc_hist, &vatrack, "VA_DistToVert10");
  }
  {
    UseVATracks vatrack;
    EfficiencyCalculation(mc_hist, &vatrack, "VA_Wide");
  }
  {
    RecoTrackClusterNumFilter clus_filter([](int t_tracks){return t_tracks > 15;});
    UseVATracks vatrack;
    clus_filter.AddRule(&vatrack);
    EfficiencyCalculation(mc_hist, &clus_filter, "VA_ClusFilter15");
  }*/

}
