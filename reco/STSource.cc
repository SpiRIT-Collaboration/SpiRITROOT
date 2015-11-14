#include "STSource.hh"

ClassImp(STSource)

STSource::STSource()
{
  fDataFile = "";
  fDecoder = NULL;
  fEventID = 0;

  fIsInitialized = kFALSE;
  fIsSeparatedData = kFALSE;
  fIsGainCalibration = kFALSE;
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

  if (fDataFile.EndsWith(".txt"))
    fIsSeparatedData = kTRUE;

  fDecoder = new STDecoderTask();
  fDecoder -> SetUseSeparatedData(fIsSeparatedData);

  if (fIsGainCalibration)
    fDecoder -> SetUseGainCalibration();

  if (!fIsSeparatedData)
    fDecoder -> AddData(fDataFile);
  else {
    std::ifstream listFile(fDataFile.Data());
    TString dataFileWithPath;
    Int_t iCobo = -1;
    while (dataFileWithPath.ReadLine(listFile)) {
      if (dataFileWithPath.Contains("s."))
        fDecoder -> AddData(dataFileWithPath, iCobo);
      else {
        iCobo++;
        fDecoder -> AddData(dataFileWithPath, iCobo);
      }
    }
  }

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
  Int_t status = fDecoder -> ReadEvent(fEventID);

  return status;
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

void STSource::SetUseGainCalibration()
{
  fIsGainCalibration = kTRUE;
}

TString STSource::GetDataFileName()
{
  return fDataFile;
}

Long64_t STSource::GetEventID()
{
  return fDecoder -> GetEventID();
}
