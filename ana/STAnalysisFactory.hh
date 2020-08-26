#ifndef STANALYSISFACTORY_HH
#define STANALYSISFACTORY_HH

#include "TXMLNode.h"
#include <cstring>
#include <string>
#include <map>

#include "FairTask.h"
#include "STConcReaderTask.hh"
#include "STModelReaderTask.hh"
#include "STCSVReaderTask.hh"
#include "STEfficiencyTask.hh"
#include "STPIDProbTask.hh"
#include "STPIDAnalysisTask.hh"
#include "STPiProbTask.hh"
#include "EfficiencyFactory.hh"
#include "STTransformFrameTask.hh"
#include "STFilterEventTask.hh"
#include "STSimpleGraphsTask.hh"
#include "STERATTask.hh"
#include "STReactionPlaneTask.hh"
#include "STDivideEventTask.hh"

class STAnalysisFactory
{
public:
  // constructor input: node of TaskList
  STAnalysisFactory(TXMLNode *node);
  STReaderTask* GetReaderTask();
  int GetEntries() { return fEntries; };
  STFilterEventTask* GetFilterEventTask();
  FairTask* GetPIDTask();
  STPiProbTask* GetPiProbTask();
  STTransformFrameTask* GetTransformFrameTask();
  STEfficiencyTask* GetEfficiencyTask();
  STERATTask* GetERATTask();
  STSimpleGraphsTask* GetSimpleGraphsTask();
  STReactionPlaneTask* GetReactionPlaneTask();
  STDivideEventTask* GetDivideEventTask();
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
  TXMLNode *fIONode = nullptr;
  TXMLNode *fTaskNode = nullptr;
  int fEntries = 0;
};

#endif
