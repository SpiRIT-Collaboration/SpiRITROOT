#include "STAnaParticleDB.hh"
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

  fSTData = new STData();
  fLogger = FairLogger::GetLogger(); 
}

STConcReaderTask::~STConcReaderTask()
{}

InitStatus STConcReaderTask::Init()
{
  STAnaParticleDB::FillTDatabasePDG();

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

  
  if(fChain -> GetBranch("EvtData"))
    fChain -> SetBranchAddress("EvtData", &fSTData);
  else
  {
    fChain -> SetBranchAddress("STData", &fSTDataArray);
    fIsTrimmedFile = true;
  }
  fChain -> SetBranchAddress("eventType", &fEventType);
  fChain -> SetBranchAddress("eventID", &fMCLoadedID);
  ioMan -> Register("STData", "ST", fData, fIsPersistence);
  ioMan -> Register("EventID", "ST", fMCEventID, fIsPersistence);
  ioMan -> Register("EventType", "ST", fEventTypeArr, fIsPersistence);

  (new ((*fMCEventID)[0]) STVectorI()) -> fElements.push_back(0);
  (new ((*fEventTypeArr)[0]) STVectorI()) -> fElements.push_back(0);
  
  return kSUCCESS;
}

void STConcReaderTask::Register()
{
  auto tree = FairRootManager::Instance()->GetOutTree();
  tree -> Branch("eventID", &fMCLoadedID);
  tree -> Branch("eventType", &fEventType);
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
  if(fIsTrimmedFile) fChain -> SetBranchAddress("STData", &fSTDataArray);
  else fChain -> SetBranchAddress("EvtData", &fSTData);
  if(fChain -> GetEntries() > fEventID)
  {
    fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Event %d", fEventID));
    fChain -> GetEntry(fEventID);
    ++fEventID;

    fData -> Delete();
    if(fIsTrimmedFile)
      new((*fData)[0]) STData(*((STData*)fSTDataArray->At(0)));
    else
      new((*fData)[0]) STData(*fSTData);
    static_cast<STVectorI*>(fMCEventID -> At(0)) -> fElements[0] = fMCLoadedID;
    static_cast<STVectorI*>(fEventTypeArr -> At(0)) -> fElements[0] = fEventType;
  }else fLogger -> Fatal(MESSAGE_ORIGIN, "Event ID exceeds the length of the TChain");
}

void STConcReaderTask::SetPersistence(Bool_t value)                                              
{ fIsPersistence = value; }
void STConcReaderTask::SetChain(TChain* chain)
{ fChain = chain; }
