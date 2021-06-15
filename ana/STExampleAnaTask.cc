#include "STExampleAnaTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairFileHeader.h"

#include "TRandom.h"
#include "STVector.hh"
#include "TDatabasePDG.h"
#include "TParameter.h"
#include "TF1.h"
#include "TH2.h"
#include "TPad.h"

#include <cmath>

ClassImp(STExampleAnaTask);

STExampleAnaTask::STExampleAnaTask()
{
  fLogger = FairLogger::GetLogger(); 
}

STExampleAnaTask::~STExampleAnaTask()
{}

InitStatus STExampleAnaTask::Init()
{
  STAnaParticleDB::FillTDatabasePDG();
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fSkip = (STVectorI*) ioMan -> GetObject("Skip");

  return kSUCCESS;
}

void STExampleAnaTask::SetParContainers()
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

void STExampleAnaTask::Exec(Option_t *opt)
{
  if(fSkip)
    if(fSkip -> fElements[0] == 1) return; // skip flag from FilterTask

  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    int pdg = fSupportedPDG[i];
    // do what you need to do with that particle
    // remember your TClonesArray should align with fSupportedPDG
  }
}

void STExampleAnaTask::FinishTask()
{
  auto outfile = FairRootManager::Instance() -> GetOutFile();
  outfile -> cd();
  // write any meta data to output
}


