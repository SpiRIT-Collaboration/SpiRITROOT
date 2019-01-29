#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/EventSelection/ChooseFolder.h"
#include "/mnt/spirit/analysis/user/tsangc/SpiRITROOT/macros/ProtonCutGeneration/AutoProtonCut.cxx"

ROOT::TSpinMutex g_mutex;

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
  SwitchRecorder(const std::vector<double>& t_cut) : cut_(t_cut) 
  { 
    if(! std::is_sorted(cut_.begin(), cut_.end())) std::cerr << "Warning: cut interveals are not sorted. This will not work\n"; 
    entries_.resize(t_cut.size());
  }; // leave 1 for overflow bin

  void Selection(std::vector<DataSink>& t_hist, int t_entry) override
  {
    if(t_entry != current_entry_ ) 
    {
      double last_entry = t_hist.back().back()[0];
      auto low = std::lower_bound(cut_.begin(), cut_.end(), last_entry);
      int index = low - cut_.begin();
      index -= 1;
      std::lock_guard<ROOT::TSpinMutex> guard(g_mutex);
      if(index < entries_.size() && index >= 0) entries_[index].push_back(t_entry);
    }
    current_entry_ = t_entry;
    this->FillData(t_hist, t_entry); 
  }

  const std::vector<int>& GetList(int t_i) { return entries_[t_i]; };

  std::unique_ptr<Rule> Clone() override { return mylib::make_unique<SwitchRecorder>(*this); };
protected:
  std::vector<double> cut_;
  static std::vector<std::vector<int>> entries_;
  int current_entry_ = -1;
};

std::vector<std::vector<int>> SwitchRecorder::entries_;


// you do not want to modify the embeded histograms
// don't pass by reference
TH2F EfficiencyGraph(TH2F t_embed, TH2F t_mc)
{
  t_embed.Divide(&t_mc);
  return t_embed;
}


void NewEfficiency(bool t_use_va, int t_sd, bool t_make_cut=true, bool inspect = false, bool pion = false)
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

  std::vector<double> cut{0, 30, 40, 50, 60, 70, 80, 110};
  std::string name = "Efficiency_cut" + std::to_string(t_sd) + "sd";
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
  if(inspect)
  {
    tree.AppendRule<DrawHit>("hits_xz");
    tree.AppendRule<DrawHit>("hits_yz", 1, 2);
  }
  tree.AppendRule<Multiplicity>("mult");
  tree.AppendSwitch("switch", cut);
  tree.AppendRule<MCEmbedReader>("MCReader", mom_low, mom_high);
  tree.AppendRule<RecoTrackClusterNumFilter>("CFilter", [](int t_tracks){return t_tracks > 20;});
  tree.AppendRule<EmbedFilter>("EFilter");
  tree.AppendRule<DistToVertex>("avert");
  tree.AppendRule<ValueCut>("acut", 20);//, (i==0)? 5 : ((i==1)? 10 : 20));
  if(t_use_va) tree.AppendRule<UseVATracks>("vatrack");
  tree.AppendRule<PID>("pid");
  tree.AppendRule<EmbedCut>("pid_cut", cut_name.Data(), (pion)? "pip":"CUTG");
  tree.AppendRule<MomentumTracks>("Mom", 3);
  tree.AppendRule<ValueCut>("MomCut", mom_low, mom_high);
  tree.AppendRule<ThetaPhi>("angular");

  tree.AppendRejectedRuleTo<ThetaPhi>("pid_cut", "Rjangular");
  tree.AppendRejectedRuleTo<CompareMCPrimary>("pid_cut", "comparemc", CompareMCPrimary::MMagDiff, CompareMCPrimary::StartMagDiff);
  
  if(inspect)
  {
    // in inspection model, the analysis will not be ran
    // on tracks from the intermediate results are shown
    tree.AppendRejectedRuleTo<ValueCut>("pid_cut", "comparemcdiff", -100, -20);
    tree.EventCheckPoint("comparemcdiff", "eventviewer");
    tree.EventCheckPoint("CFilter", "allEvent");

    TChain mctree("cbmsim");
    for(auto name : files)
    {
      name += ".mc.root";
      mctree.Add(name.c_str());
      break;
    }
    chain.AddFriend(&mctree);
    std::cout <<" Friends added\n";
    tree.EventViewer(&chain, {"allEvent", "hits", "eventviewer"});
    return;
  }

  auto embed_angular = tree.Inspect("angular", "EmbedData", "Embeded Angular distribution;#theta (deg);#phi (deg)"
                                    ,30,0,60,25,-180,180);
  auto embed_pid = tree.Inspect("pid", "PID_Embed", "PID of the embedded protons;Momentum (MeV/c);dE/dx (arb. unit)"
                                ,100,0.8*mom_low,1.2*mom_high,1000, 0,1000);
  auto rejected_angular = tree.Inspect("Rjangular", "PIDRejected_EmbedData", "PID rejected Embeded Angular distribution;#theta (deg);#phi (deg)"
                                       ,30,0,60,25,-180,180);
  auto compare_mc = tree.Inspect("comparemc", "RejectedMomRes_EmbedData", "Momentum of the rejected particles;Momentum (MeV/c2);Dist to vertex (mm)"
                                 ,200,-50,50,500,-20,20);
  auto mc_angular = tree.Inspect("MCReader", "mc_num_mult","MC Angular distribution;#theta (deg);#phi (deg)"
                                 ,30,0,60,25,-180,180);
  chain.LoadBaskets(2000000000);
  tree.DrawMultiple(&chain);

  /*************************************
  // save the results after calculation
  **************************************/

  DailyLogger logger(TString::Format("MCProtonMult_mom_%g_%g", mom_low, mom_high).Data(), name + ".root");
  for(int j = 0; j < cut.size()-1; ++j)
  {
    //auto hist = GetMCDist(chain,cut[j-1], cut[j], mom_low, mom_high);
    mc_angular[j]->SetName(TString::Format("mc_num_mult_%g_%g", cut[j], cut[j+1]).Data());
    logger.SaveClass(*mc_angular[j]);
  }
  {
    DailyLogger logger(TString::Format("Embedded_mom_%g_%g_%s", mom_low, mom_high, name.c_str()).Data(), name + ".root");
    logger.SaveClass(*cutg);
    for(int j = 0; j < cut.size()-1; ++j)
    {
      embed_angular[j]->SetName(TString::Format("EmbedData_%s_%g_%g", name.c_str(), cut[j], cut[j+1]).Data());
      embed_pid[j]->SetName(TString::Format("PID_Embed_%s_%g_%g", name.c_str(), cut[j], cut[j+1]).Data());
      rejected_angular[j]->SetName(TString::Format("PIDRejected_EmbedData_%s_%g_%g", name.c_str(), cut[j], cut[j+1]).Data());
      compare_mc[j]->SetName(TString::Format("RejectedMomRes_EmbedData_%s_%g_%g", name.c_str(), cut[j], cut[j+1]).Data());
 
      logger.SaveClass(*embed_angular[j]);
      logger.SaveClass(*embed_pid[j]);
      logger.SaveClass(*rejected_angular[j]);
      logger.SaveClass(*compare_mc[j]);
    }
  }
  for(int j = 1; j < cut.size()-1; ++j)
  {
    DailyLogger logger("Efficiency_" + name, name + ".root");
    auto efficiency = EfficiencyGraph(*embed_angular[j-1], *mc_angular[j-1]);
    auto efficiency_name = TString::Format("Efficiency_mult_%g_%g_%s", cut[j-1], cut[j], name.c_str());
    efficiency.SetName(efficiency_name.Data());
    logger.SaveClass(efficiency);
  }
  for(int j =2; j < cut.size()-1; ++j)
  {
    mc_angular[0]->Add(mc_angular[j-1].get());
    embed_angular[0]->Add(embed_angular[j-1].get());
  }
  DailyLogger logger2("Efficiency_" + name, name + ".root");
  auto efficiency = EfficiencyGraph(*embed_angular[0], *mc_angular[0]);
  efficiency.SetName("Efficiency_all_mult");
  logger2.SaveClass(efficiency);
}

void NewEfficiency()
{
  for(int use_va = 0; use_va < 2; ++use_va)
    for(int sd = 1; sd < 4; ++sd)
      NewEfficiency((use_va == 1), sd, true, false, false);
}
