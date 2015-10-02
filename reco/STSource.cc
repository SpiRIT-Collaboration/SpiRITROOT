#include "STSource.hh"

ClassImp(STSource)

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

    return kTRUE;
  }

  if (fDataFile.IsNull()) {
    LOG(FATAL) << "Data file is not set!" << FairLogger::endl;

    return kFALSE;
  }

  fDecoder = new STDecoderTask();
  fDecoder -> AddData(fDataFile);
  fDecoder -> SetFPNPedestal();
  Bool_t decoderInit = fDecoder -> Init();

  if (decoderInit == kFALSE) {
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

  return 0;
}

void STSource::Reset()
{
}

void STSource::SetEventID(Long64_t eventid)
{
  fEventID = eventid;
}
