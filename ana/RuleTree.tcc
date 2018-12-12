#include "RuleTree.hh"

template<class T, class ...Args>
std::shared_ptr<T> RuleTree::AppendRule(const std::string& t_name, Args... args)
{
  std::shared_ptr<T> rule(new T(args...));
  if(rule_list_.empty()) first_rule_ = rule;
  else first_rule_->AppendRule(rule.get());

  rule_list_.insert(std::make_pair(t_name, rule));
  return rule;
}

template<class T, class ...Args>
std::shared_ptr<T> RuleTree::AppendRejectedRuleTo(const std::string& t_insert_to, 
                                                  const std::string& t_name,
                                                  Args... args)
{
  auto it = rule_list_.find(t_insert_to);
  if(it == rule_list_.end()) throw std::runtime_error(("Rule named " + t_insert_to + " is not found").c_str());
  
  std::shared_ptr<T> rule(new T(args...));
  rule_list_.insert(std::make_pair(t_name, rule));
  it->second->AppendRejectRule(rule.get());
  return rule;
}

template<class T, class ...Args>
std::vector<std::shared_ptr<T>> RuleTree::AppendRuleToSwitch(const std::string& t_insert_to, 
                                                             const std::string& t_name,
                                                             Args... args)
{
  int i = 0;
  std::vector<std::shared_ptr<T>> rules;
  for(auto it = rule_list_.find(t_insert_to + "_" + std::to_string(i)); it != rule_list_.end();++i, it = rule_list_.find(t_insert_to + "_" + std::to_string(i)))
  {
    std::shared_ptr<T> rule(new T(args...));
    it->second->AppendRule(rule.get());
    rule_list_.insert(std::make_pair(t_name + "_" + std::to_string(i), rule));
    rules.push_back(rule);
  }
  return rules;
}

template<class T, class ...Args>
std::vector<std::shared_ptr<T>> RuleTree::AppendRejectedRuleToAll(const std::string& t_insert_to, 
                                                                  const std::string& t_name,
                                                                  Args... args)
{
  int i = 0;
  std::vector<std::shared_ptr<T>> rules;
  for(auto it = rule_list_.find(t_insert_to + "_" + std::to_string(i)); it != rule_list_.end();++i, it = rule_list_.find(t_insert_to + "_" + std::to_string(i)))
  {
    std::shared_ptr<T> rule(new T(args...));
    it->second->AppendRejectRule(rule.get());
    rule_list_.insert(std::make_pair(t_name + "_" + std::to_string(i), rule));
    rules.push_back(rule);
  }
  return rules;
}
