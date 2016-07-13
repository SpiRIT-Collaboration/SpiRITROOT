#include "STEventPreviewTask.hh"
#include "STPad.hh"
#include <fstream>

ClassImp(STEventPreviewTask)

STEventPreviewTask::STEventPreviewTask()
: STRecoTask("Event Preview Task", 1, false)
{
}

STEventPreviewTask::STEventPreviewTask(Bool_t persistence, Bool_t identifyEvent)
: STRecoTask("Event Preivew Task", 1, persistence)
{
  fIdentifyEvent = identifyEvent;
}

STEventPreviewTask::~STEventPreviewTask()
{
}

InitStatus STEventPreviewTask::Init()
{
  if (STRecoTask::Init() == kERROR)
    return kERROR;

  fRawEventArray = (TClonesArray *) fRootManager -> GetObject("STRawEvent");
  if (fRawEventArray == nullptr) {
    LOG(ERROR) << "Cannot find STRawEvent array!" << FairLogger::endl;
    return kERROR;
  }

  fEventHeader = new STEventHeader();
  fRootManager -> Register("STEventHeader", "SpiRIT", fEventHeader, fIsPersistence);

  if (fRecoHeader != nullptr) {
    TString version; {
      TString name = TString(gSystem -> Getenv("VMCWORKDIR")) + "/VERSION";
      std::ifstream vfile(name);
      vfile >> version;
      vfile.close();
    }
    fRecoHeader -> SetPar("spiritroot_version", version);
    fRecoHeader -> SetPar("pre_identifyEvent", fIdentifyEvent);
    fRootManager -> GetOutFile() -> cd();
    fRecoHeader -> Write("RecoHeader", TObject::kWriteDelete);
  }

  return kSUCCESS;
}

void STEventPreviewTask::Exec(Option_t *opt)
{
  fEventHeader -> Clear();

  STRawEvent *rawEvent = (STRawEvent *) fRawEventArray -> At(0);
  fEventHeader -> SetEventID(rawEvent -> GetEventID());

  if (fIdentifyEvent)
    LayerTest(rawEvent);

  TString status = "Unidentified Event";
       if (fEventHeader -> IsEmptyEvent())        status = "Empty Event";
  else if (fEventHeader -> IsCollisionEvent())    status = "Collision Event";
  else if (fEventHeader -> IsActiveTargetEvent()) status = "Active Target Event";
  else if (fEventHeader -> IsOffTargetEvent())    status = "Off Target Event";
  else if (fEventHeader -> IsBeamEvent())         status = "Beam Event";
  else if (fEventHeader -> IsCosmicEvent())       status = "Cosmic Event";
  else if (fEventHeader -> IsBadEvent())          status = "Bad Event";

  LOG(INFO) << "Event " << fEventHeader -> GetEventID() << " : " << status << FairLogger::endl;
}

void STEventPreviewTask::LayerTest(STRawEvent *rawEvent)
{
  Double_t charge[4] = {0};

  Int_t numPads = rawEvent -> GetNumPads();
  for (Int_t iPad = 0; iPad < numPads; iPad++) 
  {
    STPad *pad = rawEvent -> GetPad(iPad);
    if (!(pad -> GetRow() < 56 && pad -> GetRow() > 35))
      continue;

    Double_t *adc = pad -> GetADC();
    for (Int_t iTb = 100; iTb < 130; iTb++)
      charge[pad -> GetLayer()/28] += adc[iTb];
  }

  if (charge[0] > charge[1] && charge[1] > charge[2] && charge[2] > charge[3])
    fEventHeader -> SetIsCollisionEvent();
  else
    fEventHeader -> SetIsBadEvent();
}
