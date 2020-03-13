#include "STFilterEventTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TRandom.h"


ClassImp(STFilterEventTask);

STFilterEventTask::STFilterEventTask()
{
  fLogger = FairLogger::GetLogger(); 
}

STFilterEventTask::~STFilterEventTask()
{}

void STFilterEventTask::SetBeamCut(TString cutFileName, TString cutName, double acceptance)
{
  if(acceptance > 1 || acceptance < 0) fLogger -> Fatal(MESSAGE_ORIGIN, "Beam acceptance must be between 0 to 1");
  fCutFile.push_back(new TFile(cutFileName));
  fCutG.push_back((TCutG*) fCutFile.back() -> Get(cutName));
  fAcceptance.push_back(acceptance);
}

InitStatus STFilterEventTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  if(!fCutG.empty()) fLogger -> Info(MESSAGE_ORIGIN, "Beam cut is loaded from " + TString(fCutFile.back()->GetName()));
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
  
  bool fill = false;
  for(int i = 0; i < fCutG.size(); ++i)
    if(fCutG[i] -> IsInside(data -> aoq, data -> z))
    {
      if(fAcceptance[i] > gRandom->Uniform(0, 1)) fill = true;
      else fill = false;
      break;
    }
  if(fVertexCut)
    if(!(fVertexZMin < vertexz && vertexz < fVertexZMax)) fill = false;
  if(fMultCut)
    if(!(fMultMin < mult && mult < fMultMax)) fill = false;

  FairRunAna::Instance() -> MarkFill(fill);
}

void STFilterEventTask::SetBeamFor124Star(TString cutFileName)
{
  this -> SetBeamCut(cutFileName, "12147", 0.889153514);
  this -> SetBeamCut(cutFileName, "12247", 0.866398138);
  this -> SetBeamCut(cutFileName, "12148", 1);
  this -> SetBeamCut(cutFileName, "12248", 0.865077973);
  this -> SetBeamCut(cutFileName, "12348", 0.864419604);
  this -> SetBeamCut(cutFileName, "12249", 0.907971289);
  this -> SetBeamCut(cutFileName, "12349", 0.827881512);
  this -> SetBeamCut(cutFileName, "12449", 0.8673605);
  this -> SetBeamCut(cutFileName, "12350", 0.817804611);
  this -> SetBeamCut(cutFileName, "12450", 0.792812635);
  this -> SetBeamCut(cutFileName, "12550", 0.896664003);
  this -> SetBeamCut(cutFileName, "12451", 0.479354422);
  this -> SetBeamCut(cutFileName, "12551", 0.779534665);
  this -> SetBeamCut(cutFileName, "12552", 0.004694302);
  this -> SetBeamCut(cutFileName, "12652", 0.191278786);
  this -> SetBeamCut(cutFileName, "12752", 0.948961212);
  this -> SetBeamCut(cutFileName, "12753", 0.157748003);
}
