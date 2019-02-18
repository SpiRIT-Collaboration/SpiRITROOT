#include "RuleTree.hh"
#include <stdexcept>

template<class T, class ...Args>
std::vector<std::shared_ptr<T>> RuleTree::AppendRule(const std::string& t_name, Args... args)
{

  std::vector<std::shared_ptr<T>> handle;

  if(rule_list_.empty()) 
  {
    current_name_ = t_name;
    auto new_rule = std::make_shared<T>(args...);
    first_rule_ = new_rule;
    rule_list_[t_name].push_back(new_rule);
    handle.push_back(new_rule);
  }
  else
  {
    auto& current_rule = rule_list_[current_name_];
    for(auto& rule : current_rule)
    {
      auto new_rule = std::make_shared<T>(args...);
      rule->AddRule(new_rule.get());
      rule_list_[t_name].push_back(new_rule);
      handle.push_back(new_rule);
    }
    current_name_ = t_name;
  }
  rule_order_.push_back(t_name);

  return handle;
}

template<class T, class ...Args>
std::vector<std::shared_ptr<T>> RuleTree::AppendRejectedRuleTo(const std::string& t_insert_to, 
                                                               const std::string& t_name,
                                                               Args... args)
{
  auto it = rule_list_.find(t_insert_to);
  if(it == rule_list_.end()) throw std::runtime_error(("Rule named " + t_insert_to + " is not found").c_str());
  
  std::vector<std::shared_ptr<T>> handle;
  for(auto& rule : (it->second))
  {
    auto new_rule = std::make_shared<T>(args...);
    rule_list_[t_name].push_back(new_rule);
    rule->AppendRejectRule(new_rule.get());
    handle.push_back(new_rule);
  }
  return handle;
}

template<class ...Args>
std::vector<std::shared_ptr<TH2F>> RuleTree::Inspect(const std::string& t_name, 
                                                     const std::string& t_switch,
                                                     Args... args)
{
  //if(hists_.size() != current_cp_)
  //  throw std::runtime_error("Please do not mix RuleTree::Inspect and RuleTree::WireTap. Use either one of them");
  int stride = -1, steps = 1; // used for collapsing switch
  if(!t_switch.empty())
  {
    // find out where that switch is, and count number of rules that comes before and after
    auto loc_it = std::find(rule_order_.begin(), rule_order_.end(), t_switch);
    if(loc_it != rule_order_.end())
    {
      int num_rules_before = rule_list_.at(*(loc_it-1)).size();
      int num_rules_after = rule_list_.at(*(loc_it+1)).size();
      int final_size = rule_list_.at(t_name).size();
      stride = num_rules_after/num_rules_before;
      steps = final_size/num_rules_after;
    }
    else throw std::runtime_error(("Switch " + t_switch + " is not found and cannot be collapsed\n").c_str());
  }
  std::vector<std::shared_ptr<TH2F>> handles;
  int prev = current_cp_;
  this->WireTap(t_name, steps, stride);
  for(int i = prev; i < current_cp_; ++i) 
  {
    auto hist = std::make_shared<TH2F>(args...);
    hist->Sumw2(true);
    hists_.push_back(hist); 
    handles.push_back(hist);
  }

  return handles;
}
