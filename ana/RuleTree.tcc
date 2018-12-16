#include "RuleTree.hh"

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

