#include <unistd.h>

#include "waves.h"

void run_ana_waves
(
  Int_t fRunNo = 1295,
  Int_t fSplitNo = 0,
  Int_t fNumEventsInSplit = 10,
  TString fOutForm = "reco",
  Bool_t fUseMeta = kFALSE,
  TString fGainMatch = "",
  TString fDataMach = "spdaq04",
  std::vector<Int_t> fSkipEventArray = {},
  TString fMCFile = "",
  TString fPathToData = "data/", 
  TString fSupplePath = "",
  Bool_t fIsFRIBDAQ = true
)
{

  waves::outFile = TString::Format("data/waves_%04d.root", fRunNo);

  Bool_t fIsFishtank = true;
  Bool_t fIsRIKEN = false;
   cout << "running the macro" << endl;
  /* ======= This part you need initial configuration ========= */
  // Parameter database file - files should be in parameters folder.
  //TString systemDB = "systemDB.csv";
  //TString runDB = "runDB.csv";

  // Data paths - must have one %d for run number
  // If you don't need either of them, pass it blank.
  TString ggDataPathWithFormat = "";
  TString beamDataPathWithFormat = "";
  if(fIsFishtank)
     beamDataPathWithFormat = "./bdc_files/bdc_%04d.root";
  else if(fIsRIKEN)
     beamDataPathWithFormat = "";


  // Meta data path
  
  //TString fSupplePath = "/mnt/spirit/rawdata/misc/rawdataSupplement";

  // Use relative gain matching - Amplify low gain section
  Bool_t fUseGainMatching = kFALSE;
  if(fGainMatch != "")
      fUseGainMatching = kTRUE;

  /* ========================================================== */

  TString fSpiRITROOTPath = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  //TString fSystemDB = fSpiRITROOTPath + "parameters/" + systemDB;
  //TString fRunDB = fSpiRITROOTPath + "parameters/" + runDB;

  //auto fParamSetter = new STParameters(fRunNo, fSystemDB, fRunDB);
  //auto fNumEventsInRun = fParamSetter -> GetNumTotalEvents();
  //auto fParameterFile = fParamSetter -> GetParameterFile();
  auto fParameterFile = "ST.parameters.2024Spring.par";
  //auto fYPedestalOffset = fParamSetter -> GetYPedestal();
  //auto fFieldOffsetX = fParamSetter -> GetFieldOffsetX();
  //auto fFieldOffsetY = fParamSetter -> GetFieldOffsetY();
  //auto fFieldOffsetZ = fParamSetter -> GetFieldOffsetZ();
  //auto fSheetChargeDensity = fParamSetter -> GetSheetChargeDensity();
  //auto fSystemID = fParamSetter -> GetSystemID();
  //auto fTargetZ = fParamSetter -> GetTargetZ();
  auto fTargetZ = -13.2;
  //auto fBDCOffsetX = fParamSetter -> GetBDCOffsetX();
  //auto fBDCOffsetY = fParamSetter -> GetBDCOffsetY();
  auto fBDCOffsetX = -2.84168;
  auto fBDCOffsetY = -237.892;
  //auto fGGRunID = fParamSetter -> GetGGRunID();
  auto fGGRunID = 1177;
  //auto fRelativeGainRunID = fParamSetter -> GetRelativeGainRunID();

  cout << "done with pars" << endl;

  auto fIsGGDataSet = !ggDataPathWithFormat.IsNull();
  //auto fIsGGDataSet = true;
  auto fIsBeamDataSet = !beamDataPathWithFormat.IsNull();
  //TString fGGData = "ggNoiseSubtractionMacro/ggNoise_1159.root";
  TString fGGData = "";
  TString fBeamData = "";
  if (fIsGGDataSet)   fGGData = Form(ggDataPathWithFormat.Data(), fGGRunID);
  if (fIsBeamDataSet) fBeamData = Form(beamDataPathWithFormat.Data(), fRunNo);
  //TString fGainMatchingFile = fSpiRITROOTPath + Form("parameters/RelativeGainRun%d.list", fRelativeGainRunID);
  TString fGainMatchingFile = fSpiRITROOTPath + Form("parameters/%s", fGainMatch.Data());

  cout << "set gain matching file" << endl;

  Int_t start = fSplitNo * fNumEventsInSplit;
  /*if (start >= fNumEventsInRun) {
     cout << start << " is >= " << fNumEventsInRun << endl;
     return;
  }
  if (start + fNumEventsInSplit > fNumEventsInRun) {
     cout << start + fNumEventsInSplit << " is > " << fNumEventsInRun << endl;
    fNumEventsInSplit = fNumEventsInRun - start;
  }*/

  TString sRunNo   = TString::Itoa(fRunNo, 10);
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  if (fPathToData.IsNull())
    fPathToData = fSpiRITROOTPath+"macros/data/";
  else 
    gSystem->mkdir(fPathToData.Data());
  
  TString version; {
    TString name = fSpiRITROOTPath + "VERSION.compiled";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  TString par = fSpiRITROOTPath+"parameters/"+fParameterFile;
  TString geo = fSpiRITROOTPath+"geometry/geomSpiRIT.man.root";
  TString fRawDataList = "";
  fRawDataList = TString::Format("%s/list_run%04d.txt",(gSystem -> Getenv("PWD")), fRunNo);
  TString out = TString::Format("%srun%04d_s%02d.%s.root", fPathToData.Data(),fRunNo,fSplitNo,fOutForm.Data());
  TString log = TString::Format("%srun%04d_s%02d.%s.log", fPathToData.Data(),fRunNo,fSplitNo,fOutForm.Data());
  
  if (TString(gSystem -> Which(".", fRawDataList)).IsNull() && !fUseMeta) {
     //cout << "data list not found" << endl;
     if(fIsFRIBDAQ)
        gSystem -> Exec("./createList_FRIBDAQ.sh "+sRunNo);
     else  
        gSystem -> Exec("./createList.sh "+sRunNo);
  }
     //cout << "using data list " << fRawDataList << endl;

  TString fMetaDataList;
  if (fUseMeta) {
    fRawDataList = Form("%s/run_%04d/dataList.txt", fSupplePath.Data(), fRunNo);
    fMetaDataList = Form("%s/run_%04d/metadataList.txt", fSupplePath.Data(), fRunNo);
  }

  FairLogger *logger = FairLogger::GetLogger();
  //logger -> SetLogToScreen(false);

  //FairLogger::SetLogVerbosity(FairLogger::kSilent);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  STDecoderTask *decoder = new STDecoderTask();
  if(fIsFRIBDAQ)
    decoder -> SetUseFRIBDAQData();
  decoder -> SetUseSeparatedData(true);
  decoder -> SetPersistence(true);
  decoder -> SetAuxPersistence(false);
  // By default, if SetUseGainCalibration(true) is called, reading gain calibration information from parameter file.
  //decoder -> SetUseGainCalibration(true);
  /* Manual calibration parameter setters. You need to provide both calibration root file and reference values to match.
   * Use default automatic one if you're not sure what you're doing. */
  //decoder -> SetGainCalibrationData("filename.root");
  // decoder -> SetGainReference(constant, linear, quadratic);
  if (fIsGGDataSet)
    decoder -> SetGGNoiseData(fGGData);
  cout << fRawDataList << endl;
  cout << "setting data list" << endl;
  decoder -> SetDataList(fRawDataList);
  cout << "data list set" << endl;
  //decoder -> SetEventID(start);
  decoder -> SetTbRange(30, 257); 
  // Low gain calibration. Don't forget you need to uncomment PSA part, too.
  if (fUseGainMatching)
    decoder -> SetGainMatchingData(fGainMatchingFile);
  // Method to select events to reconstruct
  // Format of the input file:
  //        runid eventid
  //        runid eventid
  //        runid eventid
  //        runid eventid
  //map<Int_t, vector<Int_t> *> events;
  //string FileName_PiEvt = "./Pick_PiEvt/Sn108_PiEvt/";
  //FileName_PiEvt = FileName_PiEvt+"Sn108_Run"+fRunNo+"_PiEvt";
  //cout<<"Reading the Event list for the pion events : "<<FileName_PiEvt<<endl;
  //readEventList(FileName_PiEvt, events);
  //cout <<"Number of events " << fNumEventsInSplit << " starting at " << start <<endl;

  //decoder -> SetEventList(*events[fRunNo]);
  decoder -> SetEventID(start);

  STMacroTask *macro = new STMacroTask();
  macro -> AddInitFunction(waves::Init);
  macro -> AddFunction(waves::Exec);

  run -> AddTask(decoder);

  run -> Init();
  //run -> Run(0,1000);
  cout << "processing run " << fRunNo << " with " << fNumEventsInSplit << " events." << endl;
  run -> Run(0,fNumEventsInSplit);

  waves::Finalize();
  
  auto outFile = FairRootManager::Instance() -> GetOutFile();
  auto recoHeader = new STRecoHeader("RecoHeader","");
  recoHeader -> SetPar("version", version);
  recoHeader -> SetPar("eventStart", start);
  recoHeader -> SetPar("numEvents", fNumEventsInSplit);
  recoHeader -> SetPar("parameter", fParameterFile);
  if (fIsGGDataSet)
    recoHeader -> SetPar("GGData", fGGData);
  //recoHeader -> SetPar("yPedestal", fYPedestalOffset);
  //recoHeader -> SetPar("fieldOffsetX", fFieldOffsetX);
  //recoHeader -> SetPar("fieldOffsetY", fFieldOffsetY);
  //recoHeader -> SetPar("fieldOffsetZ", fFieldOffsetZ);
  //recoHeader -> SetPar("sheetChargeDensity", fSheetChargeDensity);
  //recoHeader -> SetPar("BDCOffsetX", fBDCOffsetX);
  //recoHeader -> SetPar("BDCOffsetY", fBDCOffsetY);
  auto driftVelocityInParameterFile = gSystem -> GetFromPipe("cat " + par + " | grep DriftVelocity | awk '{print $2}'");
  recoHeader -> SetPar("driftVelocity", driftVelocityInParameterFile);
  recoHeader -> Write("RecoHeader");

  cout << "Log    : " << log << endl;
  cout << "Input  : " << fRawDataList << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();
}

