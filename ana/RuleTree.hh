#ifndef RULETREE_H
#define RULETREE_H
#include <vector>
#include <map>
#include <string>
#include <utility>
#include <exception>

#include "RecoRules.hh"
#include "RulesDrawer.hh"
#include "TChain.h"
#include "TH1F.h"

class RuleTree
{
public:
  RuleTree(){};

  void AppendTree(RuleTree& t_tree);

  template<class T, class ...Args>
  std::vector<std::shared_ptr<T>> AppendRule(const std::string& t_name, Args... args);

  template<class T, class ...Args>
  std::vector<std::shared_ptr<T>> AppendRejectedRuleTo(const std::string& t_insert_to, 
                                          const std::string& t_name,
                                          Args... args);

  void AppendSwitch(const std::string& t_name, const std::vector<double>& t_bound, bool t_yaxis=false);
  void AppendXYSwitch(const std::string& t_name, 
                      const std::vector<double>& t_xbound, 
                      const std::vector<double>& t_ybound);


  int WireTap(const std::string& t_name, int t_id);

//private:
  std::shared_ptr<Rule> first_rule_;
  std::string current_name_; // name and suffix for switch indexing
  std::map<std::string, std::vector<std::shared_ptr<Rule>>> rule_list_;
};

#include "RuleTree.tcc"

#endif
