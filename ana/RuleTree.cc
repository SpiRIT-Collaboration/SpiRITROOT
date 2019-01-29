#include "RuleTree.hh"
#include "TCanvas.h"
#include "TGraph2D.h"

void RuleTree::AppendTree(RuleTree& t_tree)
{
  if(t_tree.first_rule_)
  {
    rule_list_.insert(t_tree.rule_list_.begin(), t_tree.rule_list_.end());
    if(rule_list_.empty()) first_rule_ = t_tree.first_rule_;
    else first_rule_->AppendRule(t_tree.first_rule_.get());
    current_name_ = t_tree.current_name_;
  }
}

void RuleTree::AppendSwitch(const std::string& t_name, const std::vector<double>& t_bound, bool t_yaxis)
{
  if(t_bound.size() < 2) std::cerr << "There must be at least 2 elements to form a bound\n";
  auto& current_rule = rule_list_[current_name_];
  for(auto& rule : current_rule)
  {
    std::shared_ptr<ValueCut> prev_cut = std::make_shared<ValueCut>(t_bound[0], t_bound[1], t_yaxis);
    rule->AddRule(prev_cut.get());
    rule_list_[t_name].push_back(prev_cut);
    for(int i = 1; i < t_bound.size()-1; ++i)
    {
      auto cut = std::make_shared<ValueCut>(t_bound[i], t_bound[i+1], t_yaxis);
      prev_cut->AppendRejectRule(cut.get());
      prev_cut = cut;
      rule_list_[t_name].push_back(cut);
    }
  }
  current_name_ = t_name;
}

void RuleTree::AppendXYSwitch(const std::string& t_name, const std::vector<double>& t_xbound, const std::vector<double>& t_ybound)
{
  if(t_xbound.size() < 2 || t_ybound.size() < 2) std::cerr << "There must be at least 2 elements to form a bound\n";
  auto& current_rule = rule_list_[current_name_];
  for(auto& rule : current_rule)
  {
    std::shared_ptr<XYCut> prev_cut = std::make_shared<XYCut>(t_xbound[0], t_xbound[1], t_ybound[0], t_ybound[1]);
    rule->AddRule(prev_cut.get());
    rule_list_[t_name].push_back(prev_cut);
    for(int j = 0; j < t_ybound.size()-1; ++j)
      for(int i = 0; i < t_xbound.size()-1; ++i)
      {
        if(i == 0 && j == 0) continue;
        auto cut = std::make_shared<XYCut>(t_xbound[i], t_xbound[i+1], t_ybound[j], t_ybound[j-1]);
        prev_cut->AppendRejectRule(cut.get());
        prev_cut = cut;
        rule_list_[t_name].push_back(cut);
      }
  }
  current_name_ = t_name;
}

int RuleTree::WireTap(const std::string& t_name)
{
  auto it = rule_list_.find(t_name);
  if(it == rule_list_.end()) throw std::runtime_error(("Rule named " + t_name + " is not found for checkpoint").c_str());
 
  for(auto& rule : it->second)
  {
    std::shared_ptr<CheckPoint> cp(new CheckPoint(current_cp_));
    rule_list_[t_name + "_cp"].push_back(cp);
    rule->InsertRule(cp.get());
    current_cp_++;
  }

  return current_cp_;
}

void RuleTree::DrawMultiple(TChain* t_chain)
{
  DrawMultipleComplex drawer(t_chain);
  drawer.DrawMultiple(*this->first_rule_.get(), hists_);
}

void RuleTree::EventViewer(TChain* t_chain, const std::vector<std::string>& t_name)
{
  std::vector<int> start_index, xz_index, yz_index;
  for(const auto& name : t_name)
  {
    start_index.push_back(current_cp_);
    this->Inspect(name + "_xz");
    xz_index.push_back(current_cp_);
    this->Inspect(name + "_yz");
    yz_index.push_back(current_cp_);
  }

  const int pad_x = 108;            
  const int pad_y = 112;
  const double size_x = 8;
  const double size_y = 12;
  const double canvas_scale = 0.5;

  t_chain->GetNtrees();
  DrawMultipleComplex drawer(t_chain);
  drawer.SetRule(this->first_rule_.get());
  TCanvas c1;
  int index = 0;
  for(const auto& list_datasink : drawer)
  {
    std::cout << "Working on Event " << index << "\r" << std::flush;
    ++index;
    std::vector<TGraph2D> graphs(t_name.size());
    bool skip_event = false;
    bool first_valid = true;
    for(int i = 0; i < t_name.size(); ++i)
    {
      auto& graph = graphs[i];
      int idx, idy;
      for(idx=start_index[i], idy=xz_index[i]; idx < xz_index[i] || idy < yz_index[i]; ++idx, ++idy)
        for(int row=0; row < list_datasink[idx].size(); ++row)
          graph.SetPoint(graph.GetN(), list_datasink[idy][row][1], list_datasink[idx][row][0], list_datasink[idy][row][0]);
      if(graph.GetN() == 0)
      {
        skip_event = true;
        break;
      }
      graph.GetYaxis()->SetRangeUser(-0.5*((double)pad_x*size_x), 0.5*((double)pad_x*size_x));
      graph.GetZaxis()->SetRangeUser(-600., 100.);
      graph.GetXaxis()->SetRangeUser(0., 1350.);
      graph.SetMarkerSize(0.5);
      graph.SetMarkerColorAlpha(i+1, 0.2); // we don't want white color
      graph.Draw((first_valid)? "P" : "P same");
      first_valid = false;
    }
    if(skip_event) continue;
    c1.Update();
    c1.Modified();
    c1.WaitPrimitive();
  }
}

void RuleTree::EventCheckPoint(const std::string& t_name, const std::string& t_cpname)
{
  auto it = rule_list_.find(t_name);
  if(it == rule_list_.end()) throw std::runtime_error(("Rule named " + t_name + " is not found for checkpoint").c_str());
 
  for(auto& rule : it->second)
  {
    std::shared_ptr<DrawTrack> xz_dt = std::make_shared<DrawTrack>();
    std::shared_ptr<DrawTrack> yz_dt = std::make_shared<DrawTrack>(1, 2);
    rule_list_[t_cpname + "_xz"].push_back(xz_dt);
    rule_list_[t_cpname + "_yz"].push_back(yz_dt);
    rule->InsertRule(xz_dt.get());
    rule->InsertRule(yz_dt.get());
  }
}
