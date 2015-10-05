#include "STSource.hh"

ClassImp(STSource)

extern Bool_t gIsInterrupted;

STSource::STSource()
{
  fDataFile = "";
  fDecoder = NULL;
  fEventID = 0;

  fIsInitialized = kFALSE;
}

Bool_t STSource::Init()
{
  if (fIsInitialized) {
    LOG(INFO) << "STSource already initialized!" << FairLogger::endl;

    return kFALSE;
  }

  if (fDataFile.IsNull()) {
    LOG(FATAL) << "Data file is not set!" << FairLogger::endl;

    return kFALSE;
  }

  fDecoder = new STDecoderTask();
  fDecoder -> AddData(fDataFile);
  fDecoder -> SetFPNPedestal();
  fDecoder -> SetParContainers();
  Bool_t decoderInit = fDecoder -> Init();

  if (decoderInit == kERROR) {
    LOG(FATAL) << "Fail to initialize STDecoderTask!" << FairLogger::endl;

    return kFALSE;
  }

  fIsInitialized = kTRUE;
  return kTRUE;
}

Int_t STSource::ReadEvent(UInt_t)
{
  fDecoder -> SetEventID(fEventID);
  fDecoder -> Exec("");
  gIsInterrupted = kTRUE;

  return 0;
}

void STSource::Reset()
{
}

void STSource::Close()
{
}

void STSource::SetData(TString filename)
{
  fDataFile = filename;
}

void STSource::SetEventID(Long64_t eventid)
{
  fEventID = eventid;
}

TString STSource::GetDataFileName()
{
  return fDataFile;
}

Long64_t STSource::GetEventID()
{
  return fDecoder -> GetEventID();
}
