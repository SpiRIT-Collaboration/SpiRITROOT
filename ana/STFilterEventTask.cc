#include "STFilterEventTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"


ClassImp(STFilterEventTask);

STFilterEventTask::STFilterEventTask()
{
  fLogger = FairLogger::GetLogger(); 
}

STFilterEventTask::~STFilterEventTask()
{}

void STFilterEventTask::SetBeamCut(TString cutFileName, TString cutName)
{
  fCutFile = new TFile(cutFileName);
  fCutG = (TCutG*) fCutFile -> Get(cutName);
}

InitStatus STFilterEventTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  if(fCutG) fLogger -> Info(MESSAGE_ORIGIN, "Beam cut is loaded from " + TString(fCutFile->GetName()));
  else fLogger -> Info(MESSAGE_ORIGIN, "No beam cut is used");

  if(fVertexCut) fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Only vertex from %f < z < %f are accepted", fVertexZMin, fVertexZMax));
  if(fMultCut) fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Only vertex from %d < z < %d are accepted", fMultMin, fMultMax));

  fData = (TClonesArray*) ioMan -> GetObject("STData");
  return kSUCCESS;
}

void STFilterEventTask::SetParContainers()
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

void STFilterEventTask::Exec(Option_t *opt)
{
  auto data = (STData*) fData -> At(0);
  auto vertexz = data -> tpcVertex.z();
  auto mult = data -> multiplicity;
  
  bool fill = true;
  if(fCutG)
    if(!fCutG -> IsInside(data -> aoq, data -> z)) fill = false;
  if(fVertexCut)
    if(!(fVertexZMin < vertexz && vertexz < fVertexZMax)) fill = false;
  if(fMultCut)
    if(!(fMultMin < mult && mult < fMultMax)) fill = false;

  FairRunAna::Instance() -> MarkFill(fill);
}
