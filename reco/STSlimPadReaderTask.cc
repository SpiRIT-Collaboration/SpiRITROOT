//-----------------------------------------------------------
// Description:
//   Reading ROOT file containing STSlimPad, which is
//   the software zero-suppressed data
//
// Author List:
//   Genie Jhang     Unemployed     (original author)
//-----------------------------------------------------------

#include "STSlimPadReaderTask.hh"
#include "STGlobal.hh"
#include "STDebugLogger.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

ClassImp(STSlimPadReaderTask);

STSlimPadReaderTask::STSlimPadReaderTask()
{
  fLogger = FairLogger::GetLogger();

  fChain = nullptr;
  fPads = nullptr;

  fNumEvents = 0;

  fGainCalib = nullptr;
  fUseGainCalibration = kFALSE;
  fGainCalibrationFile = "";
  fGainConstant = -9999;
  fGainLinear = -9999;
  fGainQuadratic = 0;

  fExternalNumTbs = kFALSE;
  fNumTbs = 512;

  fIsPersistence = kFALSE;

  fPar = NULL;
  fRawEventArray = new TClonesArray("STRawEvent");
  fRawEvent = NULL;
  fRawEventLoop = new STRawEvent();

  fEventID = -1;
}

STSlimPadReaderTask::~STSlimPadReaderTask()
{
}

void STSlimPadReaderTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STSlimPadReaderTask::SetNumTbs(Int_t numTbs)                                                   { fNumTbs = numTbs; fExternalNumTbs = kTRUE; }
void STSlimPadReaderTask::AddData(TString filename)                                                 { fDataList.push_back(filename); }
void STSlimPadReaderTask::SetUseGainCalibration(Bool_t value)                                       { fUseGainCalibration = value; }
void STSlimPadReaderTask::SetGainCalibrationData(TString filename)                                  { fGainCalibrationFile = filename; }
void STSlimPadReaderTask::SetGainReference(Double_t constant, Double_t linear, Double_t quadratic)  { fGainConstant = constant; fGainLinear = linear; fGainQuadratic = quadratic; }
void STSlimPadReaderTask::SetEventID(Long64_t eventid)                                              { fEventID = eventid; }

Long64_t STSlimPadReaderTask::GetEventID() { return fEventIDLast; }

STRawEvent *STSlimPadReaderTask::GetRawEvent(Long64_t eventid)
{
  if (eventid == -1)
    eventid = fEventID++;

  if (eventid < fNumEvents)
    fChain -> GetEntry(eventid);
  else
    return nullptr;

  fRawEventLoop -> Clear();
  fRawEventLoop -> SetEventID(fEventID);
  
  Int_t numSlimPads = fPads -> size();
  STPad normalPad;
  for (Int_t iPad = 0; iPad < numSlimPads; iPad++) {
    STSlimPad pad = fPads -> at(iPad);

    normalPad.Clear();

    Int_t row = pad.id%108;
    Int_t layer = pad.id/108;

    normalPad.SetRow(row);
    normalPad.SetLayer(layer);

    Double_t adc[512] = {0};

    Int_t numPoints = pad.tb.size();
    Int_t pointer = 0;
    for (Int_t iTb = 0; iTb < numPoints; iTb += 2) {
      for (Int_t iData = 0; iData < pad.tb.at(iTb + 1); iData++)
        adc[pad.tb.at(iTb) + iData] = pad.adc.at(pointer + iData);

      pointer += pad.tb.at(iTb + 1);
    }

    if (fUseGainCalibration)
      fGainCalib -> CalibrateADC(row, layer, fNumTbs, adc);

    for (Int_t iTb = 0; iTb < fNumTbs; iTb++)
      normalPad.SetADC(iTb, adc[iTb]);

    normalPad.SetPedestalSubtracted(kTRUE);
    fRawEventLoop -> SetPad(&normalPad);
  }

  return fRawEventLoop;
}

InitStatus
STSlimPadReaderTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");

    return kERROR;
  }

  ioMan -> Register("STRawEvent", "SPiRIT", fRawEventArray, fIsPersistence);

  fChain = new TChain("tree");
  for (Int_t iFile = 0; iFile < fDataList.size(); iFile++)
    fChain -> Add(fDataList.at(iFile));

  fNumEvents = fChain -> GetEntries(); 

  fPads = new std::vector<STSlimPad>;

  fChain -> SetBranchAddress("eventid", &fEventID);
  fChain -> SetBranchAddress("pads", &fPads);

  if (!fExternalNumTbs)
    fNumTbs = fPar -> GetNumTbs();

  fGainCalib = new STGainCalibration();
  if (fGainCalibrationFile.EqualTo("") && fUseGainCalibration == kFALSE)
    fLogger -> Info(MESSAGE_ORIGIN, "Gain not calibrated!");
  else if (fGainCalibrationFile.EqualTo("") && fUseGainCalibration == kTRUE) {
    Bool_t isSetGainCalibrationData = fGainCalib -> SetGainCalibrationData(fPar -> GetGainCalibrationDataFileName(), "f");
    if (!isSetGainCalibrationData) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find gain calibration data file!");
      
      return kERROR;
    }

    fGainCalib -> SetGainReference(fPar -> GetGCConstant(), fPar -> GetGCLinear(), fPar -> GetGCQuadratic());
    fLogger -> Info(MESSAGE_ORIGIN, "Gain calibration data is set from parameter list!");
  } else {
    Bool_t isSetGainCalibrationData = fGainCalib -> SetGainCalibrationData(fGainCalibrationFile, "f");
    if (!isSetGainCalibrationData) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find gain calibration data file!");
      
      return kERROR;
    }

    if (fGainConstant == -9999 || fGainLinear == -9999) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find gain calibration data file!");

      return kERROR;
    }

    fGainCalib -> SetGainReference(fGainConstant, fGainLinear, fGainQuadratic);
    fLogger -> Info(MESSAGE_ORIGIN, "Gain calibration data is set!");
  }

  return kSUCCESS;
}

void
STSlimPadReaderTask::SetParContainers()
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
STSlimPadReaderTask::Exec(Option_t *opt)
{
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStart("SlimPadReadTask");
#endif
  fRawEventArray -> Delete();

  if (fRawEvent == NULL)
    fRawEvent = GetRawEvent(fEventID++);

  new ((*fRawEventArray)[0]) STRawEvent(fRawEvent);

  fRawEvent = NULL;
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStop("SlimPadReadTask");
#endif
}

Int_t
STSlimPadReaderTask::ReadEvent(Int_t eventID)
{
  fRawEventArray -> Delete();

  fRawEvent = GetRawEvent(eventID);
  fEventIDLast = fEventID;

  if (fRawEvent == NULL)
    return 1;

  new ((*fRawEventArray)[0]) STRawEvent(fRawEvent);

  return 0;
}


void
STSlimPadReaderTask::FinishEvent()
{
  fRawEvent = GetRawEvent();

  if (fRawEvent == NULL)
  {
    fLogger -> Info(MESSAGE_ORIGIN, "End of file. Terminating FairRun.");
    FairRootManager::Instance() -> SetFinishRun();
  }
}
