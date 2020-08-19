#include "STConcReaderTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include <glob.h>
#include "TXMLAttr.h"

ClassImp(STConcReaderTask);

STConcReaderTask::STConcReaderTask(): fEventID(0)
{
  fChain = nullptr; 
  fData = new TClonesArray("STData");
  fMCEventID = new TClonesArray("STVectorI");
  fEventTypeArr = new TClonesArray("STVectorI");
  fRunIDArr = new TClonesArray("STVectorI");
  fMCRotZ = new STVectorF;

  fSTData = new STData();
  fLogger = FairLogger::GetLogger(); 
}

STConcReaderTask::~STConcReaderTask()
{}

void STConcReaderTask::LoadFromXMLNode(TXMLNode *node)
{
  // find out whether we are reading real data or simulated data
  // the files are named differently and thus have to be read differently
  std::string dataType(static_cast<TXMLAttr*>(node -> GetAttributes() -> At(0)) -> GetValue());
  // load node info into map
  std::map<std::string, std::vector<std::string>> nodeInfo;
  for(auto IOInfo = node -> GetChildren(); IOInfo; IOInfo = IOInfo -> GetNextNode())
    if(IOInfo -> GetNodeType() == TXMLNode::kXMLElementNode)
      nodeInfo[IOInfo -> GetNodeName()].push_back(IOInfo -> GetText());

  if(fChain) delete fChain;
  auto chain = new TChain("spirit");
  fChain = chain;

  //auto pathToData = nodeInfo["DataDir"];
  // load data. Lowered error verbose level when loading runs because some runs do not exist and have to be skipped
  auto origLevel = gErrorIgnoreLevel;
  gErrorIgnoreLevel = kFatal;

  auto firstPathToData = nodeInfo["DataDir"][0];
  if(dataType == "Real")
  {
    for(const auto& pathToData : nodeInfo["DataDir"])
    {
      int startRun = std::stoi(nodeInfo["RunFirst"][0]);
      int lastRun = std::stoi(nodeInfo["RunLast"][0]);

      for(int runNo = startRun; runNo <= lastRun ; ++runNo)
      {
        auto sRunNo = std::to_string(runNo);
        std::vector<std::string> filenamesToTry{pathToData + "run" + sRunNo + "_s*.reco.*trimmed*.root/cbmsim",
                                                pathToData + "run" + sRunNo + "_s*.reco.*conc.root/spirit",
                                                pathToData + "run" + sRunNo + "_s*.reco.*conc.root/cbmsim"};
        for(const auto& tryFilename : filenamesToTry)
          if(chain -> Add(tryFilename.c_str(), -1) > 0) // if file is successfully loaded
            fLogger -> Info(MESSAGE_ORIGIN, ("Reading from file " + tryFilename).c_str());
      }
    }
  }
  else if(dataType == "Sim")
  {
    glob_t g;
    for(const auto& pathToData : nodeInfo["DataDir"])
      for(const auto& inputName : nodeInfo["InputName"])
        glob((pathToData + inputName + ".root").c_str(), GLOB_TILDE, nullptr, &g);
    for(size_t i = 0; i < g.gl_pathc; ++i)
    {
      chain -> Add(g.gl_pathv[i]);
      fLogger -> Info(MESSAGE_ORIGIN, ("Reading from file " + std::string(g.gl_pathv[i])).c_str());
    }
    globfree(&g);
  }
  else fLogger -> Fatal(MESSAGE_ORIGIN, ("Cannot parse dataType " + dataType).c_str());

  if(chain -> GetEntries() == 0)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No entries is being read from the file!");
  gErrorIgnoreLevel = origLevel;

  fPathToData = firstPathToData;
}

int STConcReaderTask::GetNEntries()
{
  if(fChain) return fChain -> GetEntries();
  else return 0;
}

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
  if(fChain -> GetBranch("MCRotZ")) 
  {
    fChain -> SetBranchAddress("MCRotZ", &fMCRotZ);
    fLogger -> Info(MESSAGE_ORIGIN, "Retrieving MC reaction plean.");
    ioMan -> Register("MCRotZ", "ST", fMCRotZ, fIsPersistence);
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
void STConcReaderTask::SetEventID(int eventID)
{ fEventID = eventID; }
