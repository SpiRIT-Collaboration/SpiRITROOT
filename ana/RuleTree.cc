#include "RuleTree.hh"

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

int RuleTree::WireTap(const std::string& t_name, int t_id)
{
  
  auto it = rule_list_.find(t_name);
  if(it == rule_list_.end()) throw std::runtime_error(("Rule named " + t_name + " is not found for checkpoint").c_str());
 
  int index = t_id;
  for(auto& rule : it->second)
  {
    std::shared_ptr<CheckPoint> cp(new CheckPoint(index));
    rule_list_[t_name + "_cp"].push_back(cp);
    rule->InsertRule(cp.get());
    index++;
  }

  return index;
}

