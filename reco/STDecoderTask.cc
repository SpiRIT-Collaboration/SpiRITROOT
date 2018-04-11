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

ClassImp(STDecoderTask);

STDecoderTask::STDecoderTask()
{
  fLogger = FairLogger::GetLogger();

  fDecoder = NULL;
  for (Int_t iCobo = 0; iCobo < 12; iCobo++)
    fMetaData[iCobo] = "";
  fDataNum = 0;

  fFPNPedestalRMS = -1;

  fExternalNumTbs = kFALSE;
  fNumTbs = 512;

  fUseGainCalibration = kFALSE;
  fGainCalibrationFile = "";
  fGainConstant = -9999;
  fGainLinear = -9999;
  fGainQuadratic = 0;
  fGainMatchingData = "";

  fDiscardTbsBelow = 0;

  fIsPersistence = kFALSE;
  fIsEmbedding = kFALSE;
  fEmbedFile = "";
  
  fPar = NULL;
  fEmbedTrackArray = new TClonesArray("STMCTrack");
  fRawEventArray = new TClonesArray("STRawEvent");
  fRawEmbedEventArray = new TClonesArray("STRawEvent");
  fRawDataEventArray = new TClonesArray("STRawEvent");
  fRawEventMC = NULL;
  fRawEventData = new STRawEvent();
  fRawEvent = NULL;
  fChain = NULL;
  fEventArray = nullptr;
  
  fIsSeparatedData = kFALSE;

  fEventID = -1;
}

STDecoderTask::~STDecoderTask()
{
}

void STDecoderTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STDecoderTask::SetNumTbs(Int_t numTbs)                                                   { fNumTbs = numTbs; fExternalNumTbs = kTRUE; }
void STDecoderTask::AddData(TString filename, Int_t coboIdx)                                  { fDataList[coboIdx].push_back(filename); }
void STDecoderTask::SetMetaData(TString filename, Int_t coboIdx)                              { fMetaData[coboIdx] = filename; }
void STDecoderTask::SetData(Int_t value)                                                      { fDataNum = value; }
void STDecoderTask::SetFPNPedestal(Double_t pedestalRMS)                                      { fFPNPedestalRMS = pedestalRMS; }
void STDecoderTask::SetGGNoiseData(TString filename)                                          { fGGNoiseFile = filename; }
void STDecoderTask::SetUseGainCalibration(Bool_t value)                                       { fUseGainCalibration = value; }
void STDecoderTask::SetGainCalibrationData(TString filename)                                  { fGainCalibrationFile = filename; }
void STDecoderTask::SetGainReference(Double_t constant, Double_t linear, Double_t quadratic)  { fGainConstant = constant; fGainLinear = linear; fGainQuadratic = quadratic; }
void STDecoderTask::SetGainMatchingData(TString filename)                                     { fGainMatchingData = filename; };
void STDecoderTask::SetDiscardTbsBelow(Int_t tb)                                              { fDiscardTbsBelow = tb; }
void STDecoderTask::SetUseSeparatedData(Bool_t value)                                         { fIsSeparatedData = value; }
void STDecoderTask::SetEventID(Long64_t eventid)                                              { fEventID = eventid; }
void STDecoderTask::SetEmbedding(Bool_t value)                                                { fIsEmbedding = value; }
void STDecoderTask::SetEmbedFile(TString filename)                                            { fEmbedFile = filename; }

void STDecoderTask::SetDataList(TString list)
{
  std::ifstream listFile(list.Data());
  TString dataFileWithPath;
  Int_t iCobo = -1;
  while (dataFileWithPath.ReadLine(listFile)) {
    if (dataFileWithPath.Contains("s."))
      this -> AddData(dataFileWithPath, iCobo);
    else {
      iCobo++;
      this -> AddData(dataFileWithPath, iCobo);
    }
  }
}

Long64_t STDecoderTask::GetEventID() { return fEventIDLast; }

InitStatus
STDecoderTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");

    return kERROR;
  }

  //Check if embedding is turned on
  if (!fEmbedFile.EqualTo("") && fIsEmbedding)
    {
      LOG(INFO) <<"Embed is true and settingup branch adress " << FairLogger::endl;
      fChain = new TChain("cbmsim");
      fChain -> AddFile(fEmbedFile);
      fChain -> SetBranchAddress("STRawEvent", &fEventArray);
      fChain -> SetBranchAddress("STMCTrack", &fEmbedTrackArray);

      ioMan -> Register("STRawEmbedEvent", "SPiRIT", fRawEmbedEventArray, fIsPersistence);
      ioMan -> Register("STRawDataEvent", "SPiRIT", fRawDataEventArray, fIsPersistence);
      ioMan -> Register("STMCTrack", "SPiRIT", fEmbedTrackArray, fIsPersistence);
    }


  ioMan -> Register("STRawEvent", "SPiRIT", fRawEventArray, fIsPersistence);
  //  ioMan -> Register("STRawEmbedEvent", "SPiRIT", fRawEmbedEventArray, fIsPersistence);
  //  ioMan -> Register("STRawDataEvent", "SPiRIT", fRawDataEventArray, fIsPersistence);
  //  ioMan -> Register("STMCTrack", "SPiRIT", fEmbedTrackArray, fIsPersistence);
  
  fDecoder = new STCore();
  fDecoder -> SetUseSeparatedData(fIsSeparatedData);
  for (Int_t iFile = 0; iFile < fDataList[0].size(); iFile++)
    fDecoder -> AddData(fDataList[0].at(iFile));

  if (fIsSeparatedData)
    for (Int_t iCobo = 1; iCobo < 12; iCobo++)
      for (Int_t iFile = 0; iFile < fDataList[iCobo].size(); iFile++)
        fDecoder -> AddData(fDataList[iCobo].at(iFile), iCobo);

  fDecoder -> SetData(fDataNum);

  if (!fMetaData[0].IsNull()) {
    fDecoder -> LoadMetaData(fMetaData[0], 0);

    if (fIsSeparatedData)
      for (Int_t iCobo = 1; iCobo < 12; iCobo++)
        fDecoder -> LoadMetaData(fMetaData[iCobo], iCobo);
  }

  if (fExternalNumTbs)
    fDecoder -> SetNumTbs(fNumTbs);
  else
    fDecoder -> SetNumTbs(fPar -> GetNumTbs());

  fDecoder -> SetUAMap(fPar -> GetUAMapFileName());
  fDecoder -> SetAGETMap(fPar -> GetAGETMapFileName());

  if (fFPNPedestalRMS == -1)
    fFPNPedestalRMS = fPar -> GetFPNPedestalRMS();

  fDecoder -> SetFPNPedestal(fFPNPedestalRMS);

  if (!fGGNoiseFile.EqualTo("")) {
    fDecoder -> SetGGNoiseData(fGGNoiseFile);
    // This method should be after SetNumTbs() and SetFPNPedestal()
    fDecoder -> InitGGNoiseSubtractor();
  }

  if (fGainCalibrationFile.EqualTo("") && fUseGainCalibration == kFALSE)
    fLogger -> Info(MESSAGE_ORIGIN, "Gain not calibrated!");
  else if (fGainCalibrationFile.EqualTo("") && fUseGainCalibration == kTRUE) {
    Bool_t isSetGainCalibrationData = fDecoder -> SetGainCalibrationData(fPar -> GetGainCalibrationDataFileName());
    if (!isSetGainCalibrationData) {
      fLogger -> Error(MESSAGE_ORIGIN, "Cannot find gain calibration data file!");
      
      return kERROR;
    }
    LOG(INFO) << fPar -> GetGainCalibrationDataFileName() << " " << fPar -> GetGCConstant() << " " << fPar -> GetGCLinear() << " " << fPar -> GetGCQuadratic() << FairLogger::endl;

    fDecoder -> SetGainReference(fPar -> GetGCConstant(), fPar -> GetGCLinear(), fPar -> GetGCQuadratic());
    fLogger -> Info(MESSAGE_ORIGIN, "Gain calibration data is set from parameter list!");
  } else {
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

  if (fGainMatchingData.EqualTo(""))
    fLogger -> Info(MESSAGE_ORIGIN, "Relative gain maching is not done!");
  else
    fDecoder -> SetGainMatchingData(fGainMatchingData);

  fDecoder -> SetDiscardTbsBelow(fDiscardTbsBelow);

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
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStart("DecoderTask");
#endif
  fEmbedTrackArray -> Delete();
  fRawEventArray -> Delete();
  fRawEmbedEventArray -> Delete();
  fRawDataEventArray -> Delete();
  
  if (fRawEvent == NULL)
    {
      fRawEvent = fDecoder -> GetRawEvent(fEventID++);
      *fRawEventData = *fRawEvent;
    }
  if (fEmbedFile.EqualTo("") && fIsEmbedding){
    std::cout << cRED << "== [STDecoderTask] MC file for embedding not set!" << std::endl;
    exit(0);
  }
  else if (!fEmbedFile.EqualTo("") && fIsEmbedding){
    if (fEventID<2)
      std::cout << "== [STDecoderTask] Setting up embed mode" << std::endl;

    if((fEventID-1) < fChain->GetEntries())
    {
      fChain -> GetEntry(fEventID-1);
      fRawEventMC = (STRawEvent *) fEventArray -> At(0);
    }

    if(fRawEventMC != NULL)
      {
	Int_t numPads = fRawEvent -> GetNumPads();
	Int_t numPadsMC = fRawEventMC -> GetNumPads();  

	for (Int_t iPad = 0; iPad < numPads; iPad++) {
	  STPad *pad = fRawEvent -> GetPad(iPad);
	  Double_t *adc = pad -> GetADC();

	  for (Int_t iPadMC = 0; iPadMC < numPadsMC; iPadMC++){
	    STPad *padMC = fRawEventMC -> GetPad(iPadMC);
	    Double_t *adcMC = padMC -> GetADC();

	    if ((padMC -> GetRow() == pad -> GetRow()) &&
		(padMC -> GetLayer() == pad -> GetLayer()))
	      {
		for (Int_t iTb = 0; iTb < fPar -> GetNumTbs(); iTb++)
		  {
		    pad -> SetADC(iTb, adc[iTb]+adcMC[iTb]);

		  }
	      }
	  }
	}
      }
  }
  else {
    if (fEventID<2)
      std::cout << "== [STDecoderTask] Embedding mode DISABLED" << std::endl; 
  }
  
  new ((*fRawEventArray)[0]) STRawEvent(fRawEvent);
  new ((*fRawDataEventArray)[0]) STRawEvent(fRawEventData);
  if(fRawEventMC != NULL)
    new ((*fRawEmbedEventArray)[0]) STRawEvent(fRawEventMC);

  fRawEvent = NULL;
  fRawEventMC = NULL;
#ifdef TASKTIMER
  STDebugLogger::Instance() -> TimerStop("DecoderTask");
#endif
 
  //    LOG(INFO) <<"STDEcoder task finished " << FairLogger::endl;


}

/*
STRawEvent*
STDecoderTask::Embedding(TString dataFile, Int_t eventId)
{
  TChain *fChain = NULL;
  TClonesArray *fEventArray = nullptr;
  STRawEvent * rawEvent = nullptr;

  fChain = new TChain("cbmsim");
  fChain -> AddFile(dataFile);
  fChain -> SetBranchAddress("STRawEvent", &fEventArray);

  if(eventId < fChain->GetEntries())
    {
      fChain -> GetEntry(eventId);
      rawEvent = (STRawEvent *) fEventArray -> At(0);
    }
  
  return rawEvent;

}

TClonesArray*
STDecoderTask::GetEmbedTrack(TString dataFile, Int_t eventId)
{
  TChain *fChain = NULL;
  TClonesArray MCTrackArray;// = nullptr;

  fChain = new TChain("cbmsim");
  fChain -> AddFile(dataFile);
  fChain -> SetBranchAddress("STMCTrack", &fEmbedTrackArray);

  if(eventId < fChain->GetEntries())
    {
      fChain -> GetEntry(eventId);
      //      new ((*fEmbedTrackArray)[eventId]) STMCTrack();
    }
  
  //  return MCTrackArray;
}
*/
Int_t
STDecoderTask::ReadEvent(Int_t eventID)
{
  fRawEventArray -> Delete();

  fRawEvent = fDecoder -> GetRawEvent(eventID);
  fEventIDLast = fDecoder -> GetEventID();

  if (fRawEvent == NULL)
    return 1;

  new ((*fRawEventArray)[0]) STRawEvent(fRawEvent);

  return 0;
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
