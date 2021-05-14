#include "STImpactParameterMLTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TRandom.h"
#include "STVector.hh"
#include "TDatabasePDG.h"

#include <cmath>

ClassImp(STImpactParameterMLTask);

STImpactParameterMLTask::STImpactParameterMLTask(const std::string& model_filename) : 
    fSPIRITDIR(gSystem -> Getenv("VMCWORKDIR")), 
    fPipe("python " + fSPIRITDIR + "/MachineLearning/ImpactPara/predict.py "
          + fSPIRITDIR + "/MachineLearning/ImpactPara/" + model_filename)//lgb.pkl")
{
  fLogger = FairLogger::GetLogger(); 
  fLogger -> Info(MESSAGE_ORIGIN, ("Impact parameter determination algorithm: " + model_filename).c_str());
}

STImpactParameterMLTask::~STImpactParameterMLTask()
{
}

InitStatus STImpactParameterMLTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fAllObs = (STVectorF*) ioMan -> GetObject("AllObs");
  if(!fAllObs)
  {
    fLogger -> Warning(MESSAGE_ORIGIN, "[STImpactParameterMLTAsk] It uses output from STObsWriterTask. You MUST put this task before STImpactParameterMLTask");
    return kERROR;
  }

  fImpactParameterML = new STVectorF();
  fImpactParameterML -> fElements.push_back(0);
  ioMan -> Register("bML", "ST", fImpactParameterML, fIsPersistence);

  // startup the python script
  auto list_of_features = fPipe.GetOutput();
  // trim the string
  list_of_features.erase(list_of_features.find_last_not_of(" \t\n\r\f\v") + 1);
  if(list_of_features == "start") 
  {
    fLogger -> Info(MESSAGE_ORIGIN, "[STImpactParameterMLTAsk] You are using an old version of LightGBM that uses the default list of 7 observables.");
    list_of_features = "Mch N N(H-He) ETL ERAT Npt N(H-He)pt";
  }
  
  fLogger -> Info(MESSAGE_ORIGIN, "[STImpactParameterMLTAsk] Observables used in the algorithms are:");
  fLogger -> Info(MESSAGE_ORIGIN, list_of_features.c_str());
  std::stringstream ss(list_of_features);
  std::string temp;
  while(ss >> temp)
  {
    auto it = std::find(ObsHeader.begin(), ObsHeader.end(), temp);
    if(it == ObsHeader.end()) fLogger -> Fatal(MESSAGE_ORIGIN, ("Feature " + temp + " is not found. Abort.").c_str());
    fFeatureID.push_back(it - ObsHeader.begin());
    fFeatureMaxID = (fFeatureMaxID < fFeatureID.back())? fFeatureID.back() : fFeatureMaxID;
  }
  
  fLogger -> Info(MESSAGE_ORIGIN, "Python script for impact parameter has started up.");

  return kSUCCESS;
}

void STImpactParameterMLTask::SetParContainers()
{
  FairRunAna *run = FairRunAna::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

void STImpactParameterMLTask::Exec(Option_t *opt)
{
  std::string line;
  if(fFeatureMaxID > fAllObs -> fElements.size() - 1)
    fLogger -> Fatal(MESSAGE_ORIGIN, "LightGBM is asking for features that is not present in output of STObsWriterTask. You mush be using an old STObsWriterTask with new version of LightGBM training file. Please update your STObsWriterTask");
  for(int i : fFeatureID)
    line += std::to_string(fAllObs -> fElements[i]) + " ";
  fPipe.Input(line);
  fImpactParameterML -> fElements[0] = std::stof(fPipe.GetOutput());
}

void STImpactParameterMLTask::FinishTask()
{
  fPipe.Input("end");
}

void STImpactParameterMLTask::TrainAlgorithm(const std::string& obs_filename, const std::string& model_filename, const std::vector<ObsType>& features)
{
  std::string VMCDIR(gSystem -> Getenv("VMCWORKDIR"));
  std::string obsList = " \"";
  for(auto i = 0; i < features.size(); ++i) 
    obsList = obsList + ((i == 0)? "" : " ") + ObsHeader[static_cast<int>(features[i])];
  obsList = obsList + "\"";
  std::cout << obsList << std::endl;
  std::string command = "python " + VMCDIR + "/MachineLearning/ImpactPara/run-7.py " + obs_filename + " " + VMCDIR + "/MachineLearning/ImpactPara/" + model_filename + obsList;
  gSystem -> Exec(command.c_str());
}
