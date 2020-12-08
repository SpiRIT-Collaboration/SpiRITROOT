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
    fLogger -> Warning(MESSAGE_ORIGIN, "[STImpactParameterMLTAsk] It uses outputp from STObsWriterTask. You MUST put this task before STImpactParameterMLTask");
    return kERROR;
  }

  fImpactParameterML = new STVectorF();
  fImpactParameterML -> fElements.push_back(0);
  ioMan -> Register("bML", "ST", fImpactParameterML, fIsPersistence);

  // startup the python script
  fPipe.GetOutput();
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
  fPipe.Input(TString::Format("%f %f %f %f %f %f %f", 
                              fAllObs -> fElements[ObsType::MCh],
                              fAllObs -> fElements[ObsType::N],
                              fAllObs -> fElements[ObsType::N_H_He],
                              fAllObs -> fElements[ObsType::ET],
                              fAllObs -> fElements[ObsType::ERat],
                              fAllObs -> fElements[ObsType::Npt],
                              fAllObs -> fElements[ObsType::N_H_He_pt]).Data());

  fImpactParameterML -> fElements[0] = std::stof(fPipe.GetOutput());
}

void STImpactParameterMLTask::FinishTask()
{
  fPipe.Input("end");
}

void STImpactParameterMLTask::TrainAlgorithm(const std::string& obs_filename, const std::string& model_filename)
{
  std::string VMCDIR(gSystem -> Getenv("VMCWORKDIR"));
  std::string command = "python " + VMCDIR + "/MachineLearning/ImpactPara/run-7.py " + obs_filename + " " + VMCDIR + "/MachineLearning/ImpactPara/" + model_filename;
  gSystem -> Exec(command.c_str());
}
