#include <sstream>
#include <stdexcept>

#include "STAnalysisFactory.hh"
#include "STEfficiencyTask.hh"
#include "STPIDProbTask.hh"
#include "EfficiencyFactory.hh"
#include "STTransformFrameTask.hh"
#include "STFilterEventTask.hh"
#include "STSimpleGraphsTask.hh"
#include "STERATTask.hh"

#include "TObjString.h"
#include "TString.h"
#include "TXMLAttr.h"

  // constructor input: node of TaskList
STAnalysisFactory::STAnalysisFactory(TXMLNode *node) 
{ 
  if(std::strcmp(node -> GetNodeName(), "TaskList") != 0) 
    throw std::runtime_error("The node passed onto STAnalysisFactory is not TaskList.");

  auto child = node -> GetChildren();
  for(; child; child = child->GetNextNode())
    if(child -> GetNodeType() == TXMLNode::kXMLElementNode)
      fNodes[std::string(child -> GetNodeName())] = child;
};

FairTask* STAnalysisFactory::GetFilterEventTask()
{
  auto it = fNodes.find("EventFilterTask");
  if(it == fNodes.end()) return nullptr;
  auto child = it->second->GetChildren();

  auto settings = this -> fReadNodesToMap(child);
  auto task = new STFilterEventTask;
  task -> SetMultiplicityCut(std::stoi(settings["MultiplicityMin"]),
                             std::stoi(settings["MultiplicityMax"]),
                             std::stof(settings["MultiplicityDPOCA"]));
  return task;
}

FairTask* STAnalysisFactory::GetPIDTask()
{
  auto it = fNodes.find("PIDProbTask");
  if(it == fNodes.end()) return nullptr;

  auto node = it->second;
  std::string PIDType(this -> fReadNodesAttrToMap(node)["Type"]);

  // load config onto a map         
  auto child = node->GetChildren();
  auto settings = this -> fReadNodesToMap(child);

  if(PIDType == "Bay")
  {
    auto task = new STPIDProbTask();
    task -> SetMetaFile(settings["MetaFile"], false);
    task -> SetPIDFitFile(settings["PIDFit"]);
    return task;
  }
  else throw std::runtime_error("Cannot create PIDTask of type " + PIDType);
  return nullptr; 
}

FairTask* STAnalysisFactory::GetTransformFrameTask()
{
  auto it = fNodes.find("TransformFrameTask");
  if(it == fNodes.end()) return nullptr;
  auto child = it -> second -> GetChildren();

  auto settings = this -> fReadNodesToMap(child);
  auto task = new STTransformFrameTask();
  task -> SetDoRotation(true);
  task -> SetTargetMass(std::stoi(settings["TargetMass"]));
  task -> SetTargetThickness(std::stof(settings["TargetThickness"]));
  task -> SetEnergyLossFile(settings["EnergyLossFile"]);
  return task;
}

FairTask* STAnalysisFactory::GetEfficiencyTask()
{
  auto it = fNodes.find("EfficiencyTask");
  if(it == fNodes.end()) return nullptr;
  auto node = it -> second;
  std::string type(this -> fReadNodesAttrToMap(node)["Type"]);

  EfficiencyFromConcFactory *effFactory;
  if(type == "Lab") effFactory = new EfficiencyFromConcFactory;
  else effFactory = new EfficiencyInCMFactory();
  
  // variable for all particles
  int NClus;
  double DPoca;
  TString PhiCutStr, UnfoldingFile = "";
  bool update = false;
  int ThetaBins, MomBins;
  int PtBins, CMzBins;
  double UpscaleFactor;
  double PhaseSpaceFactor;

  auto task = new STEfficiencyTask(effFactory);
  auto child = node -> GetChildren();
  for(; child; child = child -> GetNextNode())
  {
     // first fill those particle independent variables
    this -> AssignIfNodeIs("NClus", child, NClus);
    this -> AssignIfNodeIs("DPoca", child, DPoca);
    this -> AssignIfNodeIs("Phi", child, PhiCutStr);
    this -> AssignIfNodeIs("UpscaleFactor", child, UpscaleFactor);
    this -> AssignIfNodeIs("PhaseSpaceFactor", child, PhaseSpaceFactor);
    if(type == "Lab")
    {
      this -> AssignIfNodeIs("ThetaBins", child, ThetaBins);
      this -> AssignIfNodeIs("MomBins", child, MomBins);
    }
    else
    {
      this -> AssignIfNodeIs("PtBins", child, PtBins);
      this -> AssignIfNodeIs("CMzBins", child, CMzBins);
    }
    if(std::strcmp(child -> GetNodeName(), "UpdateUnfolding") == 0)
    {
      UnfoldingFile = TString::Format("Unfolding/%s", child -> GetText());
      if(child -> HasAttributes()) update = true;
    }
  }

  if(!UnfoldingFile.IsNull()) task -> SetUnfoldingFile(UnfoldingFile, update);

  // turn PhiCut into vector of pair
  std::vector<std::pair<double, double>> PhiCuts;
  auto phiRangeList = PhiCutStr.Tokenize(",");
  for(int i = 0; i < phiRangeList -> GetEntriesFast(); ++i)
  {
    auto rangeStr = static_cast<TObjString*>(phiRangeList -> At(i)) -> GetString();
    auto range = rangeStr.Tokenize("-");
    PhiCuts.push_back({std::atof(range -> At(0) -> GetName()), std::atof(range -> At(1) -> GetName())});
  }
 
  // handle particle dependent config
  child = node -> GetChildren();
  for(; child; child = child -> GetNextNode())
    if(child->GetNodeType() == TXMLNode::kXMLElementNode)
      if(std::strcmp(child -> GetNodeName(), "Particle") == 0)     
      {
        auto particleNode = child -> GetChildren();
        auto particleInfo = this -> fReadNodesToMap(particleNode);
        std::string pname(static_cast<TXMLAttr*>(child -> GetAttributes() -> At(0)) -> GetValue());

        int pdg;
        if(pname == "p") pdg = 2212;
        if(pname == "d") pdg = 1000010020;
        if(pname == "t") pdg = 1000010030;
        if(pname == "He3") pdg = 1000020030;
        if(pname == "He4") pdg = 1000020040;
        if(pname == "He6") pdg = 1000020060;

        effFactory -> SetDataBaseForPDG(pdg, particleInfo["EffFile"]);
        auto& settings = task -> AccessSettings(pdg);
        settings.NClusters = NClus;
        settings.DPoca = DPoca;
        settings.PhiCuts = PhiCuts;
        if(type == "Lab")
        {
          settings.ThetaMin = 0; settings.ThetaMax = 90; 
          settings.NThetaBins = ThetaBins;
          settings.MomMin = std::stof(particleInfo["MomMin"]); 
          settings.MomMax = std::stof(particleInfo["MomMax"]); 
          settings.NMomBins = MomBins;
        }
        else
        {
          settings.PtMin = std::stof(particleInfo["PtMin"]);
          settings.PtMax = std::stof(particleInfo["PtMax"]);
          settings.NPtBins = PtBins;
          settings.CMzMin = std::stof(particleInfo["CMzMin"]);
          settings.CMzMax = std::stof(particleInfo["CMzMax"]);
          settings.NCMzBins = CMzBins;
        }
      }
  effFactory -> SetPhaseSpaceFactor(PhaseSpaceFactor);
  effFactory -> SetUpScalingFactor(UpscaleFactor);
  return task;
}

FairTask* STAnalysisFactory::GetERATTask()
{
  auto it = fNodes.find("ERATTask");
  if(it == fNodes.end()) return nullptr;
  auto task = new STERATTask();
  auto child = it -> second -> GetChildren();
  
  auto attr = this -> fReadNodesToMap(child);
  auto it2 = attr.find("ImpactParameterFile");
  if(it2 != attr.end()) task -> SetImpactParameterTable(it2 -> second);

  return new STERATTask;
}

FairTask* STAnalysisFactory::GetSimpleGraphsTask()
{
  auto it = fNodes.find("SimpleGraphsTask");
  if(it == fNodes.end()) return nullptr;
  auto graphTask = new STSimpleGraphsTask();
  auto child = it -> second -> GetChildren();

  auto attr = this -> fReadNodesToMap(child);
  if(attr.find("RapidityPlots") != attr.end()) graphTask -> RegisterRapidityPlots();
  if(attr.find("PIDPlots") != attr.end()) graphTask -> RegisterPIDPlots();
  
  return graphTask;
}

std::map<std::string, std::string> STAnalysisFactory::fReadNodesToMap(TXMLNode *node)
{
  std::map<std::string, std::string> settings;
  for(; node; node = node->GetNextNode())
    if(node->GetNodeType() == TXMLNode::kXMLElementNode)
    {
      if(auto text = node -> GetText()) settings[std::string(node->GetNodeName())] = node->GetText();
      else settings[std::string(node->GetNodeName())] = "";
    }
  
  return settings;
};

std::map<std::string, std::string> STAnalysisFactory::fReadNodesAttrToMap(TXMLNode *node)
{
  std::map<std::string, std::string> attrMap;
  if(node -> HasAttributes())
  {
    TIter next(node -> GetAttributes());
    while(auto attr = (TXMLAttr*) next()) attrMap[std::string(attr->GetName())] = attr->GetValue();
  }
  return attrMap;
};



