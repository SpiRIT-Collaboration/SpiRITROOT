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

TH2F GetMCDist(TChain& t_chain, double t_mult_low, double t_mult_high, double t_energy_low, double t_energy_high)
{
  TH2F mc_hist(TString::Format("mc_num_mult_%g_%g", t_mult_low, t_mult_high).Data(), "mc_num;#theta (deg);#phi (deg)", 40, 0, 40, 50, -180, 180);

  const std::string var("STEmbedTrack.STEmbedTrack.iMomentum.Phi()*180/3.1415926");
  const std::string var2("STEmbedTrack.STEmbedTrack.iMomentum.Theta()*180/3.1415926");
  const std::string var3("STEmbedTrack.STEmbedTrack.iMomentum.Mag()");
  const std::string var4("STRecoTrack@.GetEntries()");
  t_chain.Draw((var + ":" + var2 + ">>" + std::string(mc_hist.GetName())).c_str(), (std::to_string(t_energy_low/1000.) + " < " + var3 + " && " + var3 +" < " + std::to_string(t_energy_high/1000.)+ " && " + std::to_string(t_mult_low) + " < " + var4 + " && " + var4 + " < " + std::to_string(t_mult_high)).c_str(), "goff");

  return mc_hist;
}

// you do not want to modify the embeded histograms
// don't pass by reference
TGraph2D EfficiencyGraph(TH2F t_embed, TH2F t_mc)
{
  t_embed.RebinX(2);
  t_embed.RebinY(2);
  t_mc.RebinX(2);
  t_mc.RebinY(2);
  t_embed.Divide(&t_mc);
  auto xaxis = t_embed.GetXaxis();
  auto yaxis = t_embed.GetYaxis();
  TGraph2D graph;
  for(int j = 0; j < t_embed.GetNbinsX()+1; ++j)
    for(int k = 0; k < t_embed.GetNbinsY()+1; ++k)
    {
      double x = xaxis->GetBinCenter(j);
      double y = yaxis->GetBinCenter(k);
      if(y < -170 || y > 170) continue;
      graph.SetPoint(graph.GetN(), x, y, t_embed.GetBinContent(j, k));
      graph.SetPoint(graph.GetN(), x, y + 360., t_embed.GetBinContent(j, k));
      graph.SetPoint(graph.GetN(), x, y - 360., t_embed.GetBinContent(j, k));
    }
  return graph;
}


void NewEfficiency(bool t_use_va, int t_sd, bool t_make_cut=true)
{
  double mom_low = 350;
  double mom_high = 400;

  auto files = AggrateFolder(mom_low, mom_high, 0, 30, 0, 0, false, "/run2841_s*.reco.tommy_branch.1754.b185c90.root");
  TChain chain("cbmsim");
  for(const auto& name : files) 
  {
    std::cout << "Loading file: " << name << std::endl;
    chain.Add(name.c_str());
  }

  std::vector<double> mult_cut{0, 30, 40, 50, 60, 70, 80, 110};
  std::vector<TH2F> mc_hist;

  auto& cut = mult_cut;//(i==0)? mult_cut : (i==1)? sd_clust_cut : sd_num;
  std::string name = "cut" + std::to_string(t_sd) + "sd";
  if(t_use_va) name += "VAtrack";

  auto cut_name = TString::Format("Cut_%dSD.root", t_sd);
  if(t_use_va) cut_name = "VA" + cut_name;
  auto cutg = AutoProtonCut(t_sd, t_use_va);
  if(t_make_cut) cutg->SaveAs(cut_name.Data());
  

  RuleTree tree;
  tree.AppendRule<Multiplicity>("mult");
  tree.AppendSwitch("switch", cut);
  tree.AppendRule<MCEmbedReader>("MCReader", mom_low, mom_high);
  tree.AppendRule<RecoTrackClusterNumFilter>("CFilter", [](int t_tracks){return t_tracks > 20;});
  tree.AppendRule<EmbedFilter>("EFilter");
  tree.AppendRule<PID>("pid");
  tree.AppendRule<EmbedCut>("pid_cut", cut_name.Data());
  tree.AppendRule<DistToVertex>("avert");
  tree.AppendRule<ValueCut>("acut", 20);//, (i==0)? 5 : ((i==1)? 10 : 20));
  if(t_use_va) tree.AppendRule<UseVATracks>("vatrack");
  tree.AppendRule<MomentumTracks>("Mom", 3);
  tree.AppendRule<ValueCut>("MomCut", mom_low, mom_high);
  tree.AppendRule<ThetaPhi>("angular");

  tree.AppendRejectedRuleTo<ThetaPhi>("pid_cut", "Rjangular");
  tree.AppendRejectedRuleTo<CompareMCPrimary>("pid_cut", "comparemc", CompareMCPrimary::MMagDiff, CompareMCPrimary::StartMagDiff);

  int index = tree.WireTap("angular", 0);
  index = tree.WireTap("pid", index);
  index = tree.WireTap("Rjangular", index);
  index = tree.WireTap("comparemc", index);
  tree.WireTap("MCReader", index);
  
  std::vector<TH2F> angular_and_pid;
  for(int j = 0; j < cut.size()-1; ++j)
    angular_and_pid.emplace_back(TString::Format("EmbedData_%s_%g_%g", name.c_str(), cut[j], cut[j+1]).Data(), 
                                 "Embeded Angular distribution;#theta (deg);#phi (deg)"
                                 ,40,0,40,50,-180,180);
  for(int j = 0; j < cut.size()-1; ++j)
    angular_and_pid.emplace_back(TString::Format("PID_Embed_%s_%g_%g", name.c_str(), cut[j], cut[j+1]).Data(), 
                                 "PID of the embedded protons;Momentum (MeV/c);dE/dx (arb. unit)"
                                 ,100,0.8*mom_low,1.2*mom_high,1000, 0,1000);
  for(int j = 0; j < cut.size()-1; ++j)
    angular_and_pid.emplace_back(TString::Format("PIDRejected_EmbedData_%s_%g_%g", name.c_str(), cut[j], cut[j+1]).Data(), 
                                 "PID rejected Embeded Angular distribution;#theta (deg);#phi (deg)"
                                 ,40,0,40,50,-180,180);
  for(int j = 0; j < cut.size()-1; ++j)
    angular_and_pid.emplace_back(TString::Format("RejectedMomRes_EmbedData_%s_%g_%g", name.c_str(), cut[j], cut[j+1]).Data(), 
                                 "Momentum of the rejected particles;Momentum (MeV/c2);Dist to vertex (mm)"
                                 ,200,-50,50,500,-20,20);
  for(int j = 0; j < cut.size()-1; ++j)
    angular_and_pid.emplace_back(TString::Format("mc_num_mult_%g_%g", cut[j], cut[j+1]).Data(), 
                                 "MC Angular distribution;#theta (deg);#phi (deg)"
                                 ,40,0,40,50,-180,180);
 
  // enable error bar calculation for all histograms
  for(auto& hist : angular_and_pid) hist.Sumw2(true);

  DrawMultipleComplex drawer(&chain);
  drawer.DrawMultiple(*tree.first_rule_.get(), angular_and_pid);

  // save the results after calculation
  DailyLogger logger(TString::Format("MCProtonMult_mom_%g_%g", mom_low, mom_high).Data(), name + ".root");
  for(int j = 0; j < cut.size()-1; ++j)
  {
    //auto hist = GetMCDist(chain,cut[j-1], cut[j], mom_low, mom_high);
    auto& hist = angular_and_pid[j+index];
    logger.SaveClass(hist);
    mc_hist.push_back(hist);
  }
  {
    DailyLogger logger(TString::Format("Embedded_mom_%g_%g_%s", mom_low, mom_high, name.c_str()).Data(), name + ".root");
    logger.SaveClass(*cutg);
    for(auto& hist : angular_and_pid) logger.SaveClass(hist);
  }
  for(int j = 1; j < cut.size()-1; ++j)
  {
    DailyLogger logger("Efficiency_" + name, name + ".root");
    auto efficiency = EfficiencyGraph(angular_and_pid[j-1], mc_hist[j-1]);
    auto efficiency_name = TString::Format("Efficiency_mult_%g_%g_%s", cut[j-1], cut[j], name.c_str());
    efficiency.SetName(efficiency_name.Data());
    logger.SaveClass(efficiency);
  }
  for(int j =2; j < cut.size()-1; ++j)
  {
    mc_hist[0].Add(&mc_hist[j-1]);
    angular_and_pid[0].Add(&angular_and_pid[j-1]);
  }
  DailyLogger logger2("Efficiency_" + name, name + ".root");
  auto efficiency = EfficiencyGraph(angular_and_pid[0], mc_hist[0]);
  efficiency.SetName("Efficiency_all_mult");
  logger2.SaveClass(efficiency);
}

void NewEfficiency()
{
  for(int use_va = 0; use_va < 2; ++use_va)
    for(int sd = 1; sd < 4; ++sd)
      NewEfficiency((use_va == 0), sd);
}
