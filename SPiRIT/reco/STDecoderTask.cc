//-----------------------------------------------------------
// Description:
//   Converting GRAW file to tree structure to make it easy
//   to access the data
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#include "STDecoderTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRun.h"
#include "FairRuntimeDb.h"

ClassImp(STDecoderTask);

STDecoderTask::STDecoderTask()
{
  fLogger = FairLogger::GetLogger();

  fDecoder = NULL;
  fDataNum = 0;

  fPedestalFile = "";
  fNumTbs = 512;

  fGainCalibrationFile = "";
  fGainConstant = -9999;
  fGainSlope = -9999;

  fSignalDelayFile = "";

  fIsPersistence = kFALSE;

  fPar = NULL;
  fRawEventArray = new TClonesArray("STRawEvent");
}

STDecoderTask::~STDecoderTask()
{
}

void STDecoderTask::SetPersistence(Bool_t value)                   { fIsPersistence = value; }
void STDecoderTask::SetNumTbs(Int_t numTbs)                        { fNumTbs = numTbs; }
void STDecoderTask::AddData(TString filename)                      { fDataList.push_back(filename); }
void STDecoderTask::SetData(Int_t value)                           { fDataNum = value; }
void STDecoderTask::SetPedestalData(TString filename)              { fPedestalFile = filename; }
void STDecoderTask::SetGainCalibrationData(TString filename)       { fGainCalibrationFile = filename; }
void STDecoderTask::SetGainBase(Double_t constant, Double_t slope) { fGainConstant = constant; fGainSlope = slope; }
void STDecoderTask::SetSignalDelayData(TString filename)           { fSignalDelayFile = filename; }

InitStatus
STDecoderTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");

    return kERROR;
  }

  ioMan -> Register("STRawEvent", "SPiRIT", fRawEventArray, fIsPersistence);

  fDecoder = new STCore();
  for (Int_t iFile = 0; iFile < fDataList.size(); iFile++)
    fDecoder -> AddData(fDataList.at(iFile));
  fDecoder -> SetData(fDataNum);
  fDecoder -> SetNumTbs(fNumTbs);
  fDecoder -> SetUAMap((fPar -> GetFile(0)).Data());
  fDecoder -> SetAGETMap((fPar -> GetFile(1)).Data());
  if (fPedestalFile.EqualTo("")) {
    fLogger -> Info(MESSAGE_ORIGIN, "Use internal pedestal!");

    fDecoder -> SetInternalPedestal();
  } else {
    Bool_t isSetPedestalData = fDecoder -> SetPedestalData(fPedestalFile);
    if (!isSetPedestalData) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find pedestal data file!");
      
      return kERROR;
    }

    fLogger -> Info(MESSAGE_ORIGIN, "Pedestal data is set!");
  }

  if (fGainCalibrationFile.EqualTo(""))
    fLogger -> Info(MESSAGE_ORIGIN, "Gain not calibrated!");
  else {
    Bool_t isSetGainCalibrationData = fDecoder -> SetGainCalibrationData(fGainCalibrationFile);
    if (!isSetGainCalibrationData) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find gain calibration data file!");
      
      return kERROR;
    }

    if (fGainConstant == -9999 || fGainSlope == -9999) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find gain calibration data file!");

      return kERROR;
    }

    fDecoder -> SetGainBase(fGainConstant, fGainSlope);
    fLogger -> Info(MESSAGE_ORIGIN, "Gain calibration data is set!");
  }

  if (fSignalDelayFile.EqualTo(""))
    fLogger -> Info(MESSAGE_ORIGIN, "Signal not delayed!");
  else {
    Bool_t isSetSignalDelayData = fDecoder -> SetSignalDelayData(fSignalDelayFile);
    if (!isSetSignalDelayData) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find signal delay data file!");
      
      return kERROR;
    }

    fLogger -> Info(MESSAGE_ORIGIN, "Signal delay data is set!");
  }

  return kSUCCESS;
}

void
STDecoderTask::SetParContainers()
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
STDecoderTask::Exec(Option_t *opt)
{
  fRawEventArray -> Delete();

  STRawEvent *rawEvent = fDecoder -> GetRawEvent();
  
  new ((*fRawEventArray)[0]) STRawEvent(rawEvent);
}
