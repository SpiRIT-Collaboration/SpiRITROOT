#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/EventSelection/ChooseFolder.h"

class TotClusNum : public RecoTrackRule
{
public:
  void Selection(std::vector<DataSink>& t_hist, int t_entry) override
  {
    double tot_clus_num = (double) track_->GetClustNum(0.6);
    t_hist.push_back({{tot_clus_num, 1.}});
    this->FillData(t_hist, t_entry);
  };
  std::unique_ptr<Rule> Clone() override { return mylib::make_unique<TotClusNum>(*this);};
};

class NumShadowed : public RecoTrackRule
{
public:
  void Selection(std::vector<DataSink>& t_hist, int t_entry) override
  {
    double shadowed_num = (double) (track_->GetClustNum(0.6) - track_->GetdEdxPointArray()->size());
    t_hist.push_back({{shadowed_num, 1.}});
    this->FillData(t_hist, t_entry);
  };
  std::unique_ptr<Rule> Clone() override { return mylib::make_unique<NumShadowed>(*this);};
};

class Multiplicity : public Rule
{
public:
  void SetMyReader(TTreeReader& t_reader) override
  { myTrackArray_ = std::make_shared<ReaderValue>(t_reader, "STRecoTrack"); };

  void Selection(std::vector<DataSink>& t_hist, int t_entry) override
  {
    double mult = (double) ((*myTrackArray_)->GetEntries());
    t_hist.push_back({{mult, 1.}});
    this->FillData(t_hist, t_entry);
  };

  std::unique_ptr<Rule> Clone() override { return mylib::make_unique<Multiplicity>(*this); };
protected:
  std::shared_ptr<ReaderValue> myTrackArray_;
};

class SwitchRecorder : public Rule
{
public:
  SwitchRecorder(const std::vector<double>& t_cut) : cut_(t_cut), entries_(t_cut.size()) 
  { if(! std::is_sorted(cut_.begin(), cut_.end())) std::cerr << "Warning: cut interveals are not sorted. This will not work\n"; }; // leave 1 for overflow bin

  void Selection(std::vector<DataSink>& t_hist, int t_entry) override
  {
    if(t_entry != current_entry_ ) 
    {
      double last_entry = t_hist.back().back()[0];
      auto low = std::lower_bound(cut_.begin(), cut_.end(), last_entry);
      int index = low - cut_.begin();
      index -= 1;
      if(index < entries_.size() && index >= 0) entries_[index].push_back(t_entry);
    }
    current_entry_ = t_entry;
    this->FillData(t_hist, t_entry); 
  }

  const std::vector<int>& GetList(int t_i) { return entries_[t_i]; };

  std::unique_ptr<Rule> Clone() override { return mylib::make_unique<SwitchRecorder>(*this); };
protected:
  std::vector<double> cut_;
  std::vector<std::vector<int>> entries_;
  int current_entry_ = -1;
};

/*TH2F GetMCDist(double t_mult_low=30, double t_mult_high=40)
{
  // find real mc numbers
  auto files = AggrateFolder(350, 450, 0, 30, 0, 0, false, ".mc.root");
  TChain chain("cbmsim");
  for(const auto& name : files) 
  {
    std::cout << "Loading " << name << std::endl;
    chain.Add(name.c_str());
  }

  TEventList list("list", "lsit");
  std::ifstream file(TString::Format("EventList/EventList_mult_%g_%g.dat", t_mult_low, t_mult_high).Data());
  int id;
  while(file >> id) list.Enter(id);
  chain.SetEventList(&list);
  

  TH2F mc_hist(TString::Format("mc_num_mult_%g_%g", t_mult_low, t_mult_high).Data(), "mc_num;#theta (deg);#phi (deg)", 40, 0, 40, 50, -180, 180);

  const std::string var("TMath::ATan2(STEmbedTrack.STEmbedTrack.iMomentum.fY, STEmbedTrack.STEmbedTrack.iMomentum.fX)*180/3.1415926");
  const std::string var2("atan(sqrt(STEmbedTrack.STEmbedTrack.iMomentum.fX*STEmbedTrack.STEmbedTrack.iMomentum.fX + STEmbedTrack.STEmbedTrack.iMomentum.fY*STEmbedTrack.STEmbedTrack.iMomentum.fY)/STEmbedTrack.STEmbedTrack.iMomentum.fZ)*180/3.1415926");
  const std::string var3("sqrt(STEmbedTrack.STEmbedTrack.iMomentum.fZ*STEmbedTrack.STEmbedTrack.iMomentum.fZ + STEmbedTrack.STEmbedTrack.iMomentum.fY*STEmbedTrack.STEmbedTrack.iMomentum.fY + STEmbedTrack.STEmbedTrack.iMomentum.fX*STEmbedTrack.STEmbedTrack.iMomentum.fX)");
  chain.Draw((var + ":" + var2 + ">>" + std::string(mc_hist.GetName())).c_str(), ("0.35<" + var3 + " && " + var3 +" < 0.4").c_str(), "goff");


  return mc_hist;
}*/

TH2F GetMCDist(TChain& t_chain, double t_mult_low=30, double t_mult_high=40)
{
  // find real mc numbers
  TEventList list("list", "lsit");
  std::ifstream file(TString::Format("EventList/EventList_mult_%g_%g.dat", t_mult_low, t_mult_high).Data());
  int id;
  while(file >> id) list.Enter(id);
  t_chain.SetEventList(&list);
  
  TH2F mc_hist(TString::Format("mc_num_mult_%g_%g", t_mult_low, t_mult_high).Data(), "mc_num;#theta (deg);#phi (deg)", 40, 0, 40, 50, -180, 180);

  const std::string var("TMath::ATan2(STEmbedTrack.STEmbedTrack.iMomentum.fY, STEmbedTrack.STEmbedTrack.iMomentum.fX)*180/3.1415926");
  const std::string var2("atan(sqrt(STEmbedTrack.STEmbedTrack.iMomentum.fX*STEmbedTrack.STEmbedTrack.iMomentum.fX + STEmbedTrack.STEmbedTrack.iMomentum.fY*STEmbedTrack.STEmbedTrack.iMomentum.fY)/STEmbedTrack.STEmbedTrack.iMomentum.fZ)*180/3.1415926");
  const std::string var3("sqrt(STEmbedTrack.STEmbedTrack.iMomentum.fZ*STEmbedTrack.STEmbedTrack.iMomentum.fZ + STEmbedTrack.STEmbedTrack.iMomentum.fY*STEmbedTrack.STEmbedTrack.iMomentum.fY + STEmbedTrack.STEmbedTrack.iMomentum.fX*STEmbedTrack.STEmbedTrack.iMomentum.fX)");
  t_chain.Draw((var + ":" + var2 + ">>" + std::string(mc_hist.GetName())).c_str(), ("0.2 < " + var3 + " && " + var3 +" < 0.25").c_str(), "goff");


  return mc_hist;
}

TH2F GetClusterDist(double t_numclus_low, double t_numclus_high)
{
  auto files = AggrateFolder(350, 450, 0, 30, 0, 0, false, ".reco.mc.root");
  TChain chain("cbmsim");
  for(const auto& name : files) 
  {
    std::cout << "Loading file: " << name << std::endl;
    chain.Add(name.c_str());
  }

  TH2F mc_hist(TString::Format("mc_numclus_%g_%g", t_numclus_low, t_numclus_high).Data(), "mc_num;#theta (deg);#phi (deg)", 40, 0, 40, 50, -180, 180);

  const std::string var("TMath::ATan2(STRecoTrack.STRecoTrack.fMomentum.fY, STRecoTrack.STRecoTrack.fMomentum.fX)*180/3.1415926");
  const std::string var2("atan(sqrt(STRecoTrack.STRecoTrack.fMomentum.fX*STRecoTrack.STRecoTrack.fMomentum.fX + STRecoTrack.STRecoTrack.fMomentum.fY*STRecoTrack.STRecoTrack.fMomentum.fY)/STRecoTrack.STRecoTrack.fMomentum.fZ)*180/3.1415926");
  const std::string var3("sqrt(STRecoTrack.STRecoTrack.fMomentum.fZ*STRecoTrack.STRecoTrack.fMomentum.fZ + STRecoTrack.STRecoTrack.fMomentum.fY*STRecoTrack.STRecoTrack.fMomentum.fY + STRecoTrack.STRecoTrack.fMomentum.fX*STRecoTrack.STRecoTrack.fMomentum.fX)");
  const std::string var4("STRecoTrack.STRecoTrack.fNumRowClusters + STRecoTrack.STRecoTrack.fNumLayerClusters");

  chain.Draw((var + ":" + var2 + ">>" + std::string(mc_hist.GetName())).c_str(), TString::Format("350 < %s && %s < 400 && %f < %s && %s < %f", var3.c_str(), var3.c_str(), t_numclus_low, var4.c_str(), var4.c_str(), t_numclus_high).Data(), "goff");
  return mc_hist;
}

void NewEfficiency(const std::string& t_cutname = "LowMomProtonCut.root")
{
  auto files = AggrateFolder(150, 250, 0, 30, 0, 0, false, "/run2841_s*.reco.tommy_branch.1754.b185c90.root");
  TChain chain("cbmsim");
  for(const auto& name : files) 
  {
    std::cout << "Loading file: " << name << std::endl;
    chain.Add(name.c_str());
  }

  std::vector<double> mult_cut{0, 30, 40, 50, 60, 70, 80, 110};
  std::vector<double> sd_clust_cut{0, 20, 40, 60, 80, 100};
  std::vector<double> sd_num{0, 4, 8, 12, 16, 20};

  for(int i = 0; i < 3; ++i)
  {
    auto& cut = mult_cut;//(i==0)? mult_cut : (i==1)? sd_clust_cut : sd_num;
    RuleTree tree;
    std::shared_ptr<SwitchRecorder> recorder;
    tree.AppendRule<Multiplicity>("mult");
    if(i == 0)
      recorder = tree.AppendRule<SwitchRecorder>("recorder", cut);

    tree.AppendSwitch("switch", cut);
    tree.AppendRuleToSwitch<RecoTrackClusterNumFilter>("switch", "CFilter", [](int t_tracks){return t_tracks > 20;});
    tree.AppendRuleToSwitch<EmbedFilter>("switch", "EFilter");
    tree.AppendRuleToSwitch<PID>("switch", "pid");
    tree.AppendRuleToSwitch<EmbedCut>("switch", "pid_cut", t_cutname);
    tree.AppendRuleToSwitch<DistToVertex>("switch", "avert");
    tree.AppendRuleToSwitch<ValueCut>("switch", "acut", 0, (i==0)? 5 : ((i==1)? 10 : 20));
    tree.AppendRuleToSwitch<MomentumTracks>("switch", "Mom", 3);
    tree.AppendRuleToSwitch<ValueCut>("switch", "MomCut", 200, 250);
    tree.AppendRuleToSwitch<MCThetaPhi>("switch", "angular");
  
    std::vector<TH2F> angular;
    for(int j = 0; j < cut.size()-1; ++j)
    {
      tree.WireTap("angular_" + std::to_string(j), j);
      angular.emplace_back(TString::Format("EmbedData_%s_%g_%g", ((i==0)? "dist5" : (i==1)? "dist10" : "dist20"), cut[j], cut[j+1]).Data(), 
                                           "Embeded Angular distribution;#theta (deg);#phi (deg)"
                                           ,40,0,40,50,-180,180);
    }
              
    DrawMultipleComplex drawer(&chain);
    drawer.DrawMultiple(*tree.first_rule_.get(), angular);

    if(i == 0)
      for(int j = 1; j < cut.size(); ++j)
      {
        std::ofstream file(TString::Format("EventList/EventList_mult_%g_%g.dat", cut[j-1], cut[j]).Data());
        for(int event : recorder->GetList(j-1)) file << event << "\n";
      }
  
    {
      DailyLogger logger((i==0)? "Efficiency_mom_200_250_dist5" : (i==1)? "Efficiency_mom_200_250_dist10" : "Efficiency_mom_200_250_dist20");
      for(auto& hist : angular) logger.SaveClass(hist);
    }

    if( i == 0)
    {
      DailyLogger logger((i==0)? "MCProtonMult_mom_200_250" : "MCProtonNumClus" );
      DailyLogger logger2("Efficiency_multiplicity_corrected");
      for(int j = 1; j < cut.size(); ++j)
      {
        auto hist = GetMCDist(chain,cut[j-1], cut[j]);
        logger.SaveClass(hist);
        angular[j-1].Divide(&hist);
        logger2.SaveClass(angular[j-1]);
      }
    }
  }
}
