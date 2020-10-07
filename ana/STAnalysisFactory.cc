#include <sstream>
#include <stdexcept>

#include "STAnalysisFactory.hh"

#include "TObjString.h"
#include "TString.h"
#include "TXMLAttr.h"
#include "TSystem.h"

  // constructor input: node of TaskList
STAnalysisFactory::STAnalysisFactory(TXMLNode *node) 
{ 
  for(; node; node = node->GetNextNode())
    if(node -> GetNodeType() == TXMLNode::kXMLElementNode)
    {
      if(std::strcmp(node -> GetNodeName(), "IOInfo") == 0) fIONode = node;
      if(std::strcmp(node -> GetNodeName(), "TaskList") == 0) fTaskNode = node;
    }
 
  if(!fIONode)
    throw std::runtime_error("The node passed onto STAnalysisFactory does not contain IOInfo");
  if(fTaskNode)
  {
    //throw std::runtime_error("The node passed onto STAnalysisFactory does not contain TaskList.");
    auto child = fTaskNode -> GetChildren();
    for(; child; child = child->GetNextNode())
      if(child -> GetNodeType() == TXMLNode::kXMLElementNode)
        fNodes[std::string(child -> GetNodeName())] = child;
  }
};

STReaderTask* STAnalysisFactory::GetReaderTask()
{
  std::string DataType(this -> fReadNodesAttrToMap(fIONode)["Type"]);

  if(DataType == "Perfect")
  {
    auto settings = this -> fReadNodesToMap(fIONode -> GetChildren());
    TString dir = TString::Format("%s", settings["DataDir"].c_str());
    auto reader = new STModelReaderTask(dir);
    reader -> SetBeamAndTarget(std::stoi(settings["beamA"]), std::stoi(settings["targetA"]),
                               std::stof(settings["beamEnergyPerA"]));
    // TEMPORARY. TO BE DELETED
    //reader -> RotateEvent();
    fEntries = reader -> GetNEntries();
    return reader;
  }
  else if(DataType == "ML")
  {
    auto settings = this -> fReadNodesToMap(fIONode -> GetChildren());
    TString dir = TString::Format("%s", settings["DataDir"].c_str());
    auto reader = new STCSVReaderTask(dir);
    fEntries = reader -> GetNEntries();
    return reader;
  }
  else
  {
    auto reader = new STConcReaderTask();
    reader -> LoadFromXMLNode(fIONode);
    fEntries = reader -> GetNEntries();
    return reader;
  }
}

STFilterEventTask* STAnalysisFactory::GetFilterEventTask()
{
  auto it = fNodes.find("EventFilterTask");
  if(it == fNodes.end()) return nullptr;
  auto child = it->second->GetChildren();

  auto settings = this -> fReadNodesToMap(child);
  auto task = new STFilterEventTask;
  task -> SetMultiplicityCut(std::stoi(settings["MultiplicityMin"]),
                             std::stoi(settings["MultiplicityMax"]),
                             std::stof(settings["MultiplicityDPOCA"]));
  if(!settings["ERatMin"].empty() && !settings["ERatMax"].empty())
    task -> SetERatCut(std::stof(settings["ERatMin"]), std::stof(settings["ERatMax"]));
  auto it2 = settings.find("RejectEmpty");
  if(it2 != settings.end()) task -> SetRejectEmpty(true);
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
    task -> SetMetaFile(settings["MetaFile"], (settings.find("Update") != settings.end()));
    task -> SetPIDFitFile(settings["PIDFit"]);
    return task;
  }
  else if(PIDType == "Kaneko")
  {
    auto task = new STPIDAnalysisTask();
    task -> SetBeamA(std::stoi(settings["BeamA"]));
    return task;
  }
  else throw std::runtime_error("Cannot create PIDTask of type " + PIDType);
  return nullptr; 
}

STPiProbTask* STAnalysisFactory::GetPiProbTask()
{
  auto it = fNodes.find("PiProbTask");
  if(it == fNodes.end()) return nullptr;

  auto task = new STPiProbTask;
  auto settings = this -> fReadNodesToMap(it -> second -> GetChildren());
  if(!settings["GausFile"].empty()) task -> ReadFile(settings["GausFile"].c_str());
  return task;
}

STTransformFrameTask* STAnalysisFactory::GetTransformFrameTask()
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

STEfficiencyTask* STAnalysisFactory::GetEfficiencyTask()
{
  auto it = fNodes.find("EfficiencyTask");
  if(it == fNodes.end()) return nullptr;
  auto task_node = it -> second;

  // get efficiency nodes
  std::vector<TXMLNode*> eff_nodes;
  {
    auto child = task_node -> GetChildren();
    for(; child; child = child -> GetNextNode())
      if(child->GetNodeType() == TXMLNode::kXMLElementNode)
        if(std::string(child -> GetNodeName()) == "EfficiencyGroup") eff_nodes.push_back(child);
    if(eff_nodes.size() == 0) eff_nodes.push_back(task_node); // if there are no EfficiencyGroup, then EfficiencyTask must be one implicit EfficiencyGroup
  }

  auto task = new STEfficiencyTask();
  for(auto node : eff_nodes)
  {
    std::string type(this -> fReadNodesAttrToMap(node)["Type"]);
    EfficiencyFactory *effFactory;
    if(type == "Lab") effFactory = new EfficiencyFromConcFactory;
    else if(type == "CM") effFactory = new EfficiencyInCMFactory();
    else effFactory = new OrigEfficiencyFactory();
     
    // variable for all particles
    int NClus;
    double DPoca;
    TString PhiCutStr, UnfoldingFile = "";
    bool update = false;
    int ThetaBins, MomBins;
    int PtBins, CMzBins;
    double UpscaleFactor;
    double PhaseSpaceFactor;

    auto child = node -> GetChildren();
    for(; child; child = child -> GetNextNode())
    {
       // first fill those particle independent variables
      this -> AssignIfNodeIs("NClus", child, NClus);
      this -> AssignIfNodeIs("DPoca", child, DPoca);
      this -> AssignIfNodeIs("Phi", child, PhiCutStr);
      this -> AssignIfNodeIs("UpscaleFactor", child, UpscaleFactor);
      this -> AssignIfNodeIs("PhaseSpaceFactor", child, PhaseSpaceFactor);
      if(type == "CM")
      {
        this -> AssignIfNodeIs("PtBins", child, PtBins);
        this -> AssignIfNodeIs("CMzBins", child, CMzBins);
      }
      else
      {
        this -> AssignIfNodeIs("ThetaBins", child, ThetaBins);
        this -> AssignIfNodeIs("MomBins", child, MomBins);
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
    std::vector<int> pdgList;
    for(; child; child = child -> GetNextNode())
      if(child->GetNodeType() == TXMLNode::kXMLElementNode)
        if(std::strcmp(child -> GetNodeName(), "Particle") == 0)     
        {
          auto particleNode = child -> GetChildren();
          auto particleInfo = this -> fReadNodesToMap(particleNode);
          std::string pname(static_cast<TXMLAttr*>(child -> GetAttributes() -> At(0)) -> GetValue());

          int pdg;
          if(pname == "pi+") pdg = 211;
          if(pname == "pi-") pdg = -211;
          if(pname == "p") pdg = 2212;
          if(pname == "d") pdg = 1000010020;
          if(pname == "t") pdg = 1000010030;
          if(pname == "He3") pdg = 1000020030;
          if(pname == "He4") pdg = 1000020040;
          if(pname == "He6") pdg = 1000020060;
          pdgList.push_back(pdg);

          effFactory -> SetDataBaseForPDG(pdg, particleInfo["EffFile"]);
          auto& settings = task -> AccessSettings(pdg);
          settings.NClusters = NClus;
          settings.DPoca = DPoca;
          settings.PhiCuts = PhiCuts;
          if(type == "CM")
          {
            settings.PtMin = std::stof(particleInfo["PtMin"]);
            settings.PtMax = std::stof(particleInfo["PtMax"]);
            settings.NPtBins = PtBins;
            settings.CMzMin = std::stof(particleInfo["CMzMin"]);
            settings.CMzMax = std::stof(particleInfo["CMzMax"]);
            settings.NCMzBins = CMzBins;
          }
          else
          {
            settings.ThetaMin = 0; settings.ThetaMax = 90; 
            settings.NThetaBins = ThetaBins;
            settings.MomMin = std::stof(particleInfo["MomMin"]); 
            settings.MomMax = std::stof(particleInfo["MomMax"]); 
            settings.NMomBins = MomBins;
          }

        }
    effFactory -> SetPhaseSpaceFactor(PhaseSpaceFactor);
    effFactory -> SetUpScalingFactor(UpscaleFactor);
    task -> SetFactoriesForParticle(effFactory, pdgList);
  }
  return task;
}

STERATTask* STAnalysisFactory::GetERATTask()
{
  auto it = fNodes.find("ERATTask");
  if(it == fNodes.end()) return nullptr;
  auto task = new STERATTask();
  auto child = it -> second -> GetChildren();
  
  auto attr = this -> fReadNodesToMap(child);
  auto it2 = attr.find("ImpactParameterFile");
  if(it2 != attr.end()) task -> SetImpactParameterTable(it2 -> second);

  return task;
}

STSimpleGraphsTask* STAnalysisFactory::GetSimpleGraphsTask()
{
  auto it = fNodes.find("SimpleGraphsTask");
  if(it == fNodes.end()) return nullptr;
  auto graphTask = new STSimpleGraphsTask();
  auto child = it -> second -> GetChildren();

  auto attr = this -> fReadNodesToMap(child);
  if(attr.find("RemoveParticleMin") != attr.end()) graphTask -> RemoveParticleMin();
  if(attr.find("RapidityPlots") != attr.end()) graphTask -> RegisterRapidityPlots();
  if(attr.find("PIDPlots") != attr.end()) graphTask -> RegisterPIDPlots();
  if(attr.find("VPlots") != attr.end()) graphTask -> RegisterVPlots();
  if(attr.find("PionPlots") != attr.end()) graphTask -> RegisterPionPlots();
  
  return graphTask;
}

STPhiEfficiencyTask* STAnalysisFactory::GetPhiEfficiencyTask()
{
  auto it = fNodes.find("PhiEfficiencyTask");
  if(it == fNodes.end()) return nullptr;
  auto child = it -> second -> GetChildren();

  auto settings = this -> fReadNodesToMap(child);
  auto task = new STPhiEfficiencyTask();
  task -> SetPersistence(true);
  auto it2 = settings.find("PhiEff");
  if(it2 != settings.end()) task -> LoadPhiEff(it2 -> second);
  return task;
}

STReactionPlaneTask* STAnalysisFactory::GetReactionPlaneTask()
{
  auto it = fNodes.find("ReactionPlaneTask");
  if(it == fNodes.end()) return nullptr;
  auto reactionPlaneTask = new STReactionPlaneTask();
  auto child = it -> second -> GetChildren();
 
  auto attr = this -> fReadNodesToMap(child);
  auto charge = attr["ChargeCoef"];
  if(!charge.empty()) reactionPlaneTask -> SetChargeCoef(std::stof(charge));
  auto mass = attr["MassCoef"];
  if(!mass.empty()) reactionPlaneTask -> SetMassCoef(std::stof(mass));
  auto const_coef = attr["ConstCoef"];
  if(!const_coef.empty()) reactionPlaneTask -> SetConstCoef(std::stof(const_coef));
  auto it_use_mc = attr.find("UseMCReactionPlane");
  if(it_use_mc != attr.end()) reactionPlaneTask -> UseMCReactionPlane(true, std::stof((it_use_mc -> second.empty())? "0" : it_use_mc -> second));
  auto it_bias_file = attr.find("CorrectBias");
  if(it_bias_file != attr.end())
  {
    reactionPlaneTask -> LoadBiasCorrection(it_bias_file -> second);
    reactionPlaneTask -> UseShifting();
    reactionPlaneTask -> UseFlattening();
  }
  auto part_coef = attr["ParticleCoef"];
  if(!part_coef.empty())
  {
    std::vector<double> coefs;
    std::stringstream ss(part_coef);
    while(ss.good())
    {
      std::string temp;
      std::getline(ss, temp, ','); // delimited by comma
      coefs.push_back(std::stof(temp));
    } 
    reactionPlaneTask -> SetParticleCoef(coefs);
  }

  return reactionPlaneTask;
}

STDivideEventTask* STAnalysisFactory::GetDivideEventTask()
{
  auto it = fNodes.find("DivideEventTask");
  if(it == fNodes.end()) return nullptr;
  auto divideTask = new STDivideEventTask();
  auto child = it -> second -> GetChildren();
 
  auto attr = this -> fReadNodesToMap(child);
  auto it2 = attr.find("ComplementaryTo");
  if(it2 != attr.end()) divideTask -> ComplementaryTo(it2 -> second.c_str());
  return divideTask;
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



