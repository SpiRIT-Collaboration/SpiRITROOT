#include "STConcReaderTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

ClassImp(STConcReaderTask);

STConcReaderTask::STConcReaderTask(): fEventID(0)
{
  fChain = nullptr; 
  fData = new TClonesArray("STData");
  fSTData = new STData();

  fLogger = FairLogger::GetLogger(); 
}

STConcReaderTask::~STConcReaderTask()
{}

InitStatus STConcReaderTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  if(!fChain)
  {
    fLogger -> Error(MESSAGE_ORIGIN, "TChain is NULL");
    return kERROR;
  }

  fChain -> SetBranchAddress("EvtData", &fSTData);
  ioMan -> Register("STData", "ST", fData, fIsPersistence);
  return kSUCCESS;
}

void
STConcReaderTask::SetParContainers()
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

void STConcReaderTask::Exec(Option_t *opt)
{
  if(fChain -> GetEntries() > fEventID)
  {
    fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Event %d", fEventID));
    fChain -> GetEntry(fEventID);
    ++fEventID;

    fData -> Delete();
    new((*fData)[0]) STData(*fSTData);
  }else fLogger -> Fatal(MESSAGE_ORIGIN, "Event ID exceeds the length of the TChain");
}

void STConcReaderTask::SetPersistence(Bool_t value)                                              
{ fIsPersistence = value; }
void STConcReaderTask::SetChain(TChain* chain)
{ fChain = chain; }
