#ifndef RULETREE_H
#define RULETREE_H
#include <vector>
#include <map>
#include <string>
#include <utility>
#include <exception>
#include <memory>

#include "RecoRules.hh"
#include "RulesDrawer.hh"
#include "TChain.h"
#include "TH1F.h"

class RuleTree
{
public:
  RuleTree(){ TH1::AddDirectory(false); };

  void AppendTree(RuleTree& t_tree);

  template<class T, class ...Args>
  std::vector<std::shared_ptr<T>> AppendRule(const std::string& t_name, Args... args);

  template<class T, class ...Args>
  std::vector<std::shared_ptr<T>> AppendRejectedRuleTo(const std::string& t_insert_to, 
                                          const std::string& t_name,
                                          Args... args);

  template<class ...Args>
  std::vector<std::shared_ptr<TH2F>> Inspect(const std::string& t_name, Args... args);

  void EventCheckPoint(const std::string& t_name, const std::string& t_cpname);
  void EventViewer(TChain* t_chain, const std::vector<std::string>& t_name);

  void AppendSwitch(const std::string& t_name, const std::vector<double>& t_bound, bool t_yaxis=false);
  void AppendXYSwitch(const std::string& t_name, 
                      const std::vector<double>& t_xbound, 
                      const std::vector<double>& t_ybound);


  int WireTap(const std::string& t_name);

  

  void DrawMultiple(TChain* t_chain);

//private:
  int current_cp_ = 0;
  std::vector<std::shared_ptr<TH2F>> hists_;
  std::shared_ptr<Rule> first_rule_;
  std::string current_name_; // name and suffix for switch indexing
  std::map<std::string, std::vector<std::shared_ptr<Rule>>> rule_list_;
};

#include "RuleTree.tcc"

#endif
