#include "STDivideEventTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include <glob.h>
#include "TXMLAttr.h"

ClassImp(STDivideEventTask);

STDivideEventTask::STDivideEventTask() : fComplementaryEvent("cbmsim")
{
  fLogger = FairLogger::GetLogger(); 
  fComplementaryID = new STVectorI;
  fID = new STVectorI;
  fSkip = new STVectorI;
}

STDivideEventTask::~STDivideEventTask()
{}

InitStatus STDivideEventTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  if(!fComplementaryFilename.IsNull())
  {
    fLogger -> Info(MESSAGE_ORIGIN, "Will load particles complementary to " + fComplementaryFilename);
    fComplementaryEvent.Add(fComplementaryFilename);
    if(fComplementaryEvent.GetEntries() == 0)
      fLogger -> Fatal(MESSAGE_ORIGIN, "No entries are loaded from the complementary file. Disable if you don't need it.");
    fComplementaryEvent.SetBranchAddress("TrackID", &fComplementaryID);
    //fComplementaryEvent.SetBranchAddress("EventID", &fCompEventID);
  }
  else fLogger -> Info(MESSAGE_ORIGIN, "Will cut every event into half randomly.");
  
  fSkip = (STVectorI*) ioMan -> GetObject("Skip");
  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  fEff = (TClonesArray*) ioMan -> GetObject("Eff");
  //fEventID = (TClonesArray*) ioMan -> GetObject("EventID");

  ioMan -> Register("TrackID", "ST", fID, fIsPersistence);

  return kSUCCESS;
}

void
STDivideEventTask::SetParContainers()
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

void STDivideEventTask::Exec(Option_t *opt)
{
  if(fSkip)
    if(fSkip -> fElements[0] == 1) return; // skip event

  fID -> fElements.clear();
  auto data = static_cast<STData*>(fData -> At(0));
  if(fComplementaryEvent.GetEntries() == 0)
  {
    for(int i = 0; i < data -> multiplicity; ++i) fID -> fElements.push_back(i);
    std::random_shuffle(fID -> fElements.begin(), fID -> fElements.end());
    int half = data -> multiplicity / 2;
    fID -> fElements.resize(half);
  }
  else
  {
    fComplementaryEvent.GetEntry(fTreeEventID);
    //auto compEventID = static_cast<STVectorI*>(fCompEventID -> At(0)) -> fElements[0];
    //auto eventID = static_cast<STVectorI*>(fEventID -> At(0)) -> fElements[0];
    //if(compEventID != eventID)
    //  fLogger -> Fatal(MESSAGE_ORIGIN, TString::Format("Event ID mismatch at treeID = %d, event ID = %d when complementary event ID = %d", fTreeEventID, eventID, compEventID));
    std::vector<int> all_ids;
    for(int i = 0; i < data -> multiplicity; ++i) all_ids.push_back(i);
    std::set_difference(all_ids.begin(), all_ids.end(), fComplementaryID -> fElements.begin(), fComplementaryID -> fElements.end(),
                        std::inserter(fID -> fElements, fID -> fElements.begin()));
    ++fTreeEventID;
  }

  std::sort(fID -> fElements.begin(), fID -> fElements.end());
  int order = 0;
  for(int id : fID -> fElements)
  {
    data -> recoMom[order] = data -> recoMom[id];
    data -> recoPosPOCA[order] = data -> recoPosPOCA[id];
    data -> recoPosTargetPlane[order] = data -> recoPosTargetPlane[id];
    data -> recodpoca[order] = data -> recodpoca[id];
    data -> recoNRowClusters[order] = data -> recoNRowClusters[id];
    data -> recoNLayerClusters[order] = data -> recoNLayerClusters[id];
    data -> recoCharge[order] = data -> recoCharge[id];
    data -> recoEmbedTag[order] = data -> recoEmbedTag[id];
    data -> recodedx[order] = data -> recodedx[id];

    data -> vaMom[order] = data -> vaMom[id];
    data -> vaPosPOCA[order] = data -> vaPosPOCA[id];
    data -> vaPosTargetPlane[order] = data -> vaPosTargetPlane[id];
    data -> vadpoca[order] = data -> vadpoca[id];
    data -> vaNRowClusters[order] = data -> vaNRowClusters[id];
    data -> vaNLayerClusters[order] = data -> vaNLayerClusters[id];
    data -> vaCharge[order] = data -> vaCharge[id];
    data -> vaEmbedTag[order] = data -> vaEmbedTag[id];
    data -> vadedx[order] = data -> vadedx[id];
    ++order;
  }

  int mult = fID -> fElements.size();
  data -> multiplicity = mult;
  data -> vaMultiplicity = mult;
  data -> recoMom.resize(mult);
  data -> recoPosPOCA.resize(mult);
  data -> recoPosTargetPlane.resize(mult);
  data -> recodpoca.resize(mult);
  data -> recoNRowClusters.resize(mult);
  data -> recoNLayerClusters.resize(mult);
  data -> recoCharge.resize(mult);
  data -> recoEmbedTag.resize(mult);
  data -> recodedx.resize(mult);

  data -> vaMom.resize(mult);
  data -> vaPosPOCA.resize(mult);
  data -> vaPosTargetPlane.resize(mult);
  data -> vadpoca.resize(mult);
  data -> vaNRowClusters.resize(mult);
  data -> vaNLayerClusters.resize(mult);
  data -> vaCharge.resize(mult);
  data -> vaEmbedTag.resize(mult);
  data -> vadedx.resize(mult);

  if(fProb)
  {
    for(int i = 0; i < fSupportedPDG.size(); ++i)
    {
      order = 0;
      auto prob = static_cast<STVectorF*>(fProb -> At(i));
      for(int id : fID -> fElements) 
      {
        prob -> fElements[order] = prob -> fElements[id];
        ++order;
      }
      prob -> fElements.resize(mult);
    }
  }

  if(fEff)
  {
    for(int i = 0; i < fSupportedPDG.size(); ++i)
    {
      order = 0;
      auto eff = static_cast<STVectorF*>(fEff -> At(i));
      for(int id : fID -> fElements)
      {
        eff -> fElements[order] = eff -> fElements[id];
        ++order;
      }
      eff -> fElements.resize(mult);
    }
  }
  
}

void STDivideEventTask::ComplementaryTo(TString ana_filename)
{ fComplementaryFilename = ana_filename; }
void STDivideEventTask::SetPersistence(Bool_t value)                                              
{ fIsPersistence = value; }
