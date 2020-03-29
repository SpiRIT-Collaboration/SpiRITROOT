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
  fMCEventID = new TClonesArray("STVectorI");
  fEventTypeArr = new TClonesArray("STVectorI");
  fRunIDArr = new TClonesArray("STVectorI");

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
  
  if(fChain -> GetEntries() == 0)
  {
    fLogger -> Error(MESSAGE_ORIGIN, "The TChain is empty");
    return kERROR;
  }

  new ((*fData)[0]) STData();
  (new ((*fMCEventID)[0]) STVectorI()) -> fElements.push_back(0);
  (new ((*fEventTypeArr)[0]) STVectorI()) -> fElements.push_back(0);
  (new ((*fRunIDArr)[0]) STVectorI()) -> fElements.push_back(0);
  
  if(fChain -> GetBranch("EvtData"))
  {
    fChain -> SetBranchAddress("EvtData", &fSTData);
    fChain -> SetBranchAddress("eventType", &fEventType);
    fChain -> SetBranchAddress("eventID", &fMCLoadedID);
    fChain -> SetBranchAddress("runID", &fRunID);
  }
  else
  {
    fChain -> SetBranchAddress("STData", &fData);
    if(fChain -> GetBranch("EventID")) fChain -> SetBranchAddress("EventID", &fMCEventID);
    else fLogger -> Warning(MESSAGE_ORIGIN, "EventID is not found in TChain.");
    if(fChain -> GetBranch("EventType")) fChain -> SetBranchAddress("EventType", &fEventTypeArr);
    else fLogger -> Warning(MESSAGE_ORIGIN, "EventType is not found in TChain.");
    if(fChain -> GetBranch("RunID")) fChain -> SetBranchAddress("RunID", &fRunIDArr);
    else fLogger -> Warning(MESSAGE_ORIGIN, "RunID is not found in TChain.");
    fIsTrimmedFile = true;
  }

  ioMan -> Register("STData", "ST", fData, fIsPersistence);
  ioMan -> Register("EventID", "ST", fMCEventID, fIsPersistence);
  ioMan -> Register("EventType", "ST", fEventTypeArr, fIsPersistence);
  ioMan -> Register("RunID", "ST", fRunIDArr, fIsPersistence);

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

    if(!fIsTrimmedFile)
    {
      *static_cast<STData*>(fData -> At(0)) = *fSTData;
      static_cast<STVectorI*>(fMCEventID -> At(0)) -> fElements[0] = fMCLoadedID;
      static_cast<STVectorI*>(fEventTypeArr -> At(0)) -> fElements[0] = fEventType;
      static_cast<STVectorI*>(fRunIDArr -> At(0)) -> fElements[0] = fRunID;
    }
  }else fLogger -> Fatal(MESSAGE_ORIGIN, "Event ID exceeds the length of the TChain");
}

void STConcReaderTask::SetPersistence(Bool_t value)                                              
{ fIsPersistence = value; }
void STConcReaderTask::SetChain(TChain* chain)
{ fChain = chain; }
