#include "STEmbedTask.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#define cRED "\033[1;31m"
#define cYELLOW "\033[1;33m"

ClassImp(STEmbedTask);

STEmbedTask::STEmbedTask()
{
  fLogger = FairLogger::GetLogger();

  fEmbedTrackArray = new TClonesArray("STMCTrack");
  fRawEventArray = new TClonesArray("STRawEvent");
  fRawEmbedEventArray = new TClonesArray("STRawEvent");
  fRawDataEventArray = new TClonesArray("STRawEvent");
  fRawEventMC = NULL;
  fRawEventData = new STRawEvent();
  fRawEvent = NULL;
  fChain = NULL;
  fEventArray = nullptr;
  
  fEventID = -1;
}

STEmbedTask::~STEmbedTask()
{
}
void STEmbedTask::SetEventID(Long64_t eventid)                                              { fEventID = eventid; }
void STEmbedTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STEmbedTask::SetEmbedFile(TString filename)                                            { fEmbedFile = filename; }

InitStatus
STEmbedTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");

    return kERROR;
  }

  //Check if embedding is turned on
  if (!fEmbedFile.EqualTo(""))
    {
      std::cout << "== [STEmbedTask] Setting up embed mode" << std::endl;
      fChain = new TChain("cbmsim");
      fChain -> Add(fEmbedFile);
      if(fChain -> GetListOfFiles() -> GetEntries() == 0)
      {
         std::cout << "== [STEmbedTask] Embed file does not Exist!" << std::endl;
         return kERROR;
       }

      fChain -> SetBranchAddress("STRawEvent", &fEventArray);
      fChain -> SetBranchAddress("STMCTrack", &fEmbedTrackArray);

      ioMan -> Register("STRawEmbedEvent", "SPiRIT", fRawEmbedEventArray, fIsPersistence);
      ioMan -> Register("STRawDataEvent", "SPiRIT", fRawDataEventArray, fIsPersistence);
      ioMan -> Register("STMCTrack", "SPiRIT", fEmbedTrackArray, fIsPersistence);
      fPar -> SetIsEmbed(kTRUE);
    }
  else
    {
      std::cout << "== [STEmbedTask] Embedding mode DISABLED" << std::endl; 
      return kERROR;
    }

  fRawEventArray = (TClonesArray*) ioMan -> GetObject("STRawEvent");
  
  return kSUCCESS;
}

void
STEmbedTask::SetParContainers()
{
  FairRun *run = FairRun::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

void
STEmbedTask::Exec(Option_t *opt)
{
  fRawEmbedEventArray -> Delete();
  fRawDataEventArray -> Delete();

  fRawEvent = (STRawEvent*) fRawEventArray -> At(0);
  Int_t numPads = fRawEvent -> GetNumPads();
  

  new ((*fRawDataEventArray)[0]) STRawEvent(fRawEvent);
  
 
  if( (fEventID % fChain->GetEntries()) < fChain->GetEntries())
  {
    int fMCEventID = fEventID % fChain -> GetEntries();
    fChain -> GetEntry(fMCEventID);
    fRawEventMC = (STRawEvent *) fEventArray -> At(0);
  }

  
  //
  if(fRawEventMC != nullptr)
  {
    new ((*fRawEmbedEventArray)[0]) STRawEvent(fRawEventMC);
    Int_t numPadsMC = fRawEventMC -> GetNumPads();  
    // turns MC pads into a list
    // should provide a better performance in removing elements
    std::list<STPad*> MCPadsList;
    for (Int_t iPadMC = 0; iPadMC < numPadsMC; iPadMC++)
      MCPadsList.push_back(fRawEventMC -> GetPad(iPadMC));
    
    for (Int_t iPad = 0; iPad < numPads; iPad++) {
      STPad *pad = fRawEvent -> GetPad(iPad);
      Double_t *adc = pad -> GetADC();
      Int_t maxTb = fPar -> GetNumTbs();
      if(pad -> IsSaturated())
      {
        Int_t satTb = pad -> GetSaturatedTbMC();
        // MC saturatedTbMC can be infinite
        // need to check
        if(satTb < maxTb) maxTb = pad -> GetSaturatedTbMC();// only embed MC up to just before saturation point
      }
      
      for (auto padMCIt = MCPadsList.begin(); padMCIt != MCPadsList.end(); ++padMCIt){
        auto padMC = *padMCIt;
        Double_t *adcMC = padMC -> GetADC();     
        if ((padMC -> GetRow() == pad -> GetRow()) &&
          (padMC -> GetLayer() == pad -> GetLayer()))
        {
          for (Int_t iTb = 0; iTb < maxTb; iTb++)
              pad -> SetADC(iTb, adc[iTb]+adcMC[iTb]);
          MCPadsList.erase(padMCIt); 
          break;
        }
      }
    }

    // inser what's remains in the MCPadList
    for(auto padMC : MCPadsList)
      fRawEvent->SetPad(padMC);
  }

  
  fRawEventMC = nullptr;
  fEventID++;
}
