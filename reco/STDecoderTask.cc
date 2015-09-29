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
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

ClassImp(STDecoderTask);

STDecoderTask::STDecoderTask()
{
  fLogger = FairLogger::GetLogger();

  fDecoder = NULL;
  fDataNum = 0;

  fUseInternalPedestal = kFALSE;
  fPedestalStartTb = 3;
  fAverageTbs = 20;
  fPedestalFile = "";
  fPedestalRMSFactor = 0;
  fUseFPNPedestal = kFALSE;
  fFPNPedestalRMS = 5;

  fExternalNumTbs = kFALSE;
  fNumTbs = 512;

  fGainCalibrationFile = "";
  fGainConstant = -9999;
  fGainLinear = -9999;
  fGainQuadratic = 0;

  fIsPersistence = kFALSE;

  fPar = NULL;
  fRawEventArray = new TClonesArray("STRawEvent");
  fRawEvent = NULL;

  fOldData = kFALSE;

  fEventID = 0;
}

STDecoderTask::~STDecoderTask()
{
}

void STDecoderTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STDecoderTask::SetNumTbs(Int_t numTbs)                                                   { fNumTbs = numTbs; fExternalNumTbs = kTRUE; }
void STDecoderTask::AddData(TString filename)                                                 { fDataList.push_back(filename); }
void STDecoderTask::SetData(Int_t value)                                                      { fDataNum = value; }
void STDecoderTask::SetInternalPedestal(Int_t startTb, Int_t averageTbs)                      { fUseInternalPedestal = kTRUE; fPedestalStartTb = startTb; fAverageTbs = averageTbs; } 
void STDecoderTask::SetFPNPedestal(Double_t pedestalRMS)                                      { fUseFPNPedestal = kTRUE; fUseInternalPedestal = kFALSE; fPedestalFile = ""; fFPNPedestalRMS = pedestalRMS; }
void STDecoderTask::SetPedestalData(TString filename, Double_t rmsFactor)                     { fPedestalFile = filename; fPedestalRMSFactor = rmsFactor; }
void STDecoderTask::SetGainCalibrationData(TString filename)                                  { fGainCalibrationFile = filename; }
void STDecoderTask::SetGainReference(Double_t constant, Double_t linear, Double_t quadratic)  { fGainConstant = constant; fGainLinear = linear; fGainQuadratic = quadratic; }
void STDecoderTask::SetOldData(Bool_t oldData)                                                { fOldData = oldData; }

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

  if (fExternalNumTbs)
    fDecoder -> SetNumTbs(fNumTbs);
  else
    fDecoder -> SetNumTbs(fPar -> GetNumTbs());

  fDecoder -> SetOldData(fOldData);
  fDecoder -> SetUAMap((fPar -> GetFile(0)).Data());
  fDecoder -> SetAGETMap((fPar -> GetFile(1)).Data());

  if (fUseInternalPedestal)
    fDecoder -> SetInternalPedestal(fPedestalStartTb, fAverageTbs);

  if (!fPedestalFile.EqualTo("")) {
    Bool_t isSetPedestalData = fDecoder -> SetPedestalData(fPedestalFile, fPedestalRMSFactor);
    if (!isSetPedestalData) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find pedestal data file!");
      
      return kERROR;
    }

    fLogger -> Info(MESSAGE_ORIGIN, "Pedestal data is set!");
  }

  if (fUseFPNPedestal)
    fDecoder -> SetFPNPedestal(fFPNPedestalRMS);

  if (fGainCalibrationFile.EqualTo(""))
    fLogger -> Info(MESSAGE_ORIGIN, "Gain not calibrated!");
  else {
    Bool_t isSetGainCalibrationData = fDecoder -> SetGainCalibrationData(fGainCalibrationFile);
    if (!isSetGainCalibrationData) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find gain calibration data file!");
      
      return kERROR;
    }

    if (fGainConstant == -9999 || fGainLinear == -9999) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find gain calibration data file!");

      return kERROR;
    }

    fDecoder -> SetGainReference(fGainConstant, fGainLinear, fGainQuadratic);
    fLogger -> Info(MESSAGE_ORIGIN, "Gain calibration data is set!");
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

  if (fRawEvent == NULL)
    fRawEvent = fDecoder -> GetRawEvent();

  new ((*fRawEventArray)[0]) STRawEvent(fRawEvent);

  fRawEvent == NULL;
}

void
STDecoderTask::FinishEvent()
{
  fRawEvent = fDecoder -> GetRawEvent();

  if (fRawEvent == NULL)
  {
    fLogger -> Info(MESSAGE_ORIGIN, "End of file. Terminating FairRun.");
    FairRootManager::Instance() -> SetFinishRun();
  }
}
