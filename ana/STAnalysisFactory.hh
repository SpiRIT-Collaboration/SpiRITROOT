#ifndef STANALYSISFACTORY_HH
#define STANALYSISFACTORY_HH

#include "TXMLNode.h"
#include <cstring>
#include <string>
#include <map>

#include "FairTask.h"

class STAnalysisFactory
{
public:
  // constructor input: node of TaskList
  STAnalysisFactory(TXMLNode *node);
  FairTask* GetFilterEventTask();
  FairTask* GetPIDTask();
  FairTask* GetTransformFrameTask();
  FairTask* GetEfficiencyTask();

private:
  template<typename T>
  bool AssignIfNodeIs(const std::string& name, TXMLNode *node, T& value)
  {
    if(node->GetNodeType() == TXMLNode::kXMLElementNode)
      if(std::strcmp(node -> GetNodeName(), name.c_str()) == 0)
      {
        std::stringstream ss(node -> GetText());
        if(ss >> value) return true;
      }
    return false;
  };

  std::map<std::string, std::string> fReadNodesToMap(TXMLNode *node);
  std::map<std::string, std::string> fReadNodesAttrToMap(TXMLNode *node);
  std::map<std::string, TXMLNode*> fNodes;
};

#endif
