#include "RuleTree.hh"

void RuleTree::AppendTree(RuleTree& t_tree)
{
  if(t_tree.first_rule_)
  {
    rule_list_.insert(t_tree.rule_list_.begin(), t_tree.rule_list_.end());
    if(rule_list_.empty()) first_rule_ = t_tree.first_rule_;
    else first_rule_->AppendRule(t_tree.first_rule_.get());
  }
}

void RuleTree::AppendSwitch(const std::string& t_name, const std::vector<double>& t_bound, bool t_yaxis)
{
  if(t_bound.size() < 2) std::cerr << "There must be at least 2 elements to form a bound\n";
  this->AppendRule<ValueCut>(t_name + "_0", t_bound[0], t_bound[1], t_yaxis);
  for(int i = 1; i < t_bound.size()-1; ++i)
    this->AppendRejectedRuleTo<ValueCut>(t_name + "_" + std::to_string(i-1),
                                         t_name + "_" + std::to_string(i), 
                                         t_bound[i], t_bound[i+1], t_yaxis);
}

void RuleTree::AppendXYSwitch(const std::string& t_name, const std::vector<double>& t_xbound, const std::vector<double>& t_ybound)
{
  if(t_xbound.size() < 2 || t_ybound.size() < 2) std::cerr << "There must be at least 2 elements to form a bound\n";
  this->AppendRule<XYCut>(t_name + "_0", t_xbound[0], t_xbound[1], t_ybound[0], t_ybound[1]);
  int index = 1;
  for(int j = 0; j < t_ybound.size()-1; ++j)
    for(int i = 0; i < t_xbound.size()-1; ++i)
    {
      if(i == 0 && j == 0) continue;
      this->AppendRejectedRuleTo<XYCut>(t_name + "_" + std::to_string(index-1),
                                        t_name + "_" + std::to_string(index), 
                                        t_xbound[i], t_xbound[i+1],
                                        t_ybound[j], t_ybound[j+1]);
      ++index;
    }
}

void RuleTree::WireTap(const std::string& t_name, int t_id)
{
  std::shared_ptr<CheckPoint> cp(new CheckPoint(t_id));
  auto it = rule_list_.find(t_name);
  if(it == rule_list_.end()) throw std::runtime_error(("Rule named " + t_name + " is not found for checkpoint").c_str());

  rule_list_.insert(std::make_pair(t_name + "_cp", cp));
  it->second->InsertRule(cp.get());
}

