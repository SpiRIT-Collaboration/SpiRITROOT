#include <unistd.h>

void run_reco_2024
(
  Int_t fRunNo = 1295,
  Int_t fSplitNo = 0,
  Int_t fNumEventsInSplit = 10,
  TString fOutForm = "reco.2024",
  Bool_t fUseMeta = kFALSE,
  TString fGainMatch = "",
  TString fDataMach = "spdaq04",
  std::vector<Int_t> fSkipEventArray = {},
  TString fMCFile = "",
  TString fPathToData = "", 
  TString fSupplePath = "",
  Bool_t fIsFRIBDAQ = true
)
{
  Bool_t fIsFishtank = true;
  Bool_t fIsRIKEN = false;
   cout << "running the macro" << endl;
  /* ======= This part you need initial configuration ========= */
  // Parameter database file - files should be in parameters folder.
  //TString systemDB = "systemDB.csv";
  //TString runDB = "runDB.csv";

  // Data paths - must have one %d for run number
  // If you don't need either of them, pass it blank.
  TString ggDataPathWithFormat = "ggNoiseSubtractionMacro/ggNoise%d.root";
  TString beamDataPathWithFormat = "";
  if(fIsFishtank)
     beamDataPathWithFormat = "/mnt/cephfs/hira/SPIRIT_2024/beam_VETO_KATANA_data/bdc_rootfiles/bdc_%d.root";
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
     //else if(fIsFRIBDAQ && fDataMach == "spdaq01")
        //gSystem -> Exec("./createList_FRIBDAQ_spdaq01.sh "+sRunNo);
     //else if(fIsFRIBDAQ) {
        //cout << "ERROR: " << fDataMach << " does not match a known data location." << endl << "Aborting!" << endl;
        //return;
     //}
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
 
  
  if (fUseMeta) 
  {
    std::ifstream metalistFile(fMetaDataList.Data());
    TString dataFileWithPath;
    for (Int_t iCobo = 0; iCobo < 12; iCobo++) {
      dataFileWithPath.ReadLine(metalistFile);
      dataFileWithPath = Form("%s/run_%04d/%s", fSupplePath.Data(), fRunNo, dataFileWithPath.Data());
      decoder -> SetMetaData(dataFileWithPath, iCobo);
    }
  }

  auto embedTask = new STEmbedTask();
  embedTask -> SetPersistence(false);
  embedTask -> SetEventID(start);
  embedTask -> SetEmbedFile(fMCFile);

  auto preview = new STEventPreviewTask();
  preview -> SetSkippingEvents(fSkipEventArray);
  preview -> SetPersistence(true);
  //preview -> SetSelectingEvents(*events[fRunNo]);

  auto psa = new STPSAETask();
  psa -> SetPersistence(true);
  // In case pulse cut threshold need to be changed. (Default: 30)
  // psa -> SetThreshold(value);
  // By default, it uses all the layers.
  // psa -> SetLayerCut(-1, 112);
  // Pulse having long tail
  psa -> SetPulserData("pulser_117ns_50tb.dat");
  // Default: Rensheng's peak finding method 1. Finds small peaks
  // psa -> SetPSAPeakFindingOption(0);
  // Low gain calibration. Don't forget you need to uncomment decoder part, too.
  if (fUseGainMatching)
    psa -> SetGainMatchingData(fGainMatchingFile);
  // Statistically matching time jitter in different CoBos
  psa -> SetYOffsets(fSpiRITROOTPath + "parameters/yOffsetCalibration.dat");
  // This is used to match the TPC-Vertex_Y with the BDC_Y.
  //psa -> SetYPedestalOffset(fYPedestalOffset); // unit: mm

  auto helix = new STHelixTrackingTask();
  helix -> SetPersistence(true);
  helix -> SetClusterPersistence(true);
  // Left, right, top and bottom sides cut
  helix -> SetClusterCutLRTB(420, -420, -64, -522);
  // High density region cut
  helix -> SetEllipsoidCut(TVector3(0, -260, -11.9084), TVector3(120, 100, 220), 5); // current use
  // Changing clustering direction angle and margin. Default: 45 deg with 0 deg margin
  // helix -> SetClusteringAngleAndMargin(35., 3.);

  auto genfitPID = new STGenfitPIDTask();
  // In the TPC frame. Here the z position is used when Genfit do the extrapolation.
  genfitPID -> SetTargetPlane(0, 0, fTargetZ); // unit: mm
  genfitPID -> SetPersistence(true);
  genfitPID -> SetBDCFile("");
  // Only for test
  // genfitPID -> SetConstantField();
  genfitPID -> SetListPersistence(true);
  // Removing shorter length tracklet by distance of adjacent clusters.
  // genfitPID -> SetMaxDCluster(60);

  auto genfitVA = new STGenfitVATask();
  genfitVA -> SetIs2024Data();
  genfitVA -> SetPersistence(true);
  // Only for test
  // genfitVA -> SetConstantField();
  genfitVA -> SetListPersistence(true);
  if (fIsBeamDataSet) {
    genfitVA -> SetBeamFile(fBeamData);
    genfitVA -> SetInformationForBDC(fRunNo, fBDCOffsetX, fBDCOffsetY, 0);
  }
  // Uncomment if you want to recalculate the vertex using refit tracks.
  genfitVA -> SetUseRave(true);
  //genfitVA -> ShiftBDCAfterSC("../parameters/vertex_shift_Sn" + std::to_string(int(fSystemID/1000)) + ".root", 80);
  genfitVA -> SetZtoProject(-21.3, 2.24, 3); //(Double_t peakZ, Double_t sigma, Double_t sigmaMultiple), this function will project the BDC on the Target.

  run -> AddTask(decoder);
  if(!fMCFile.IsNull())
    run -> AddTask(embedTask);
  run -> AddTask(preview);
  run -> AddTask(psa);
  run -> AddTask(helix);
  run -> AddTask(genfitPID);
  run -> AddTask(genfitVA);

  run -> Init();
  //run -> Run(0,1000);
  cout << "processing run " << fRunNo << " with " << fNumEventsInSplit << " events." << endl;
  run -> Run(0,fNumEventsInSplit);
  
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

