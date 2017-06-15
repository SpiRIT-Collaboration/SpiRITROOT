void run_reco_experiment
(
  Int_t fRunNo = 3193,
  Int_t fNumEventsInRun = 10000,
  Int_t fSplitNo = 0,
  Int_t fNumEventsInSplit = 1000,
  TString fGCData = "",
  TString fGGData = "",
  Double_t fPSAThreshold = 30,
  TString fParameterFile = "ST.parameters.Commissioning_201604.par",
  TString fPathToData = "",
  Bool_t fUseMeta = kTRUE,
  TString fSupplePath = "/mnt/spirit/rawdata/misc/rawdataSupplement"
)
{
  Int_t start = fSplitNo * fNumEventsInSplit;
  if (start >= fNumEventsInRun) return;
  if (start + fNumEventsInSplit > fNumEventsInRun)
    fNumEventsInSplit = fNumEventsInRun - start;

  TString sRunNo   = TString::Itoa(fRunNo, 10);
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToData.IsNull())
    fPathToData = spiritroot+"macros/data/";
  TString version; {
    TString name = spiritroot + "VERSION";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }
  TString par = spiritroot+"parameters/"+fParameterFile;
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root";
  TString raw = TString(gSystem -> Getenv("PWD"))+"/list_run"+sRunNo+".txt";
  TString out = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco."+version+".root";
  TString log = fPathToData+"run"+sRunNo+"_s"+sSplitNo+"."+version+".log";

  if (TString(gSystem -> Which(".", raw)).IsNull() && !fUseMeta)
    gSystem -> Exec("./createList.sh "+sRunNo);

  TString metaFile;
  if (fUseMeta) {
    raw = Form("%s/run_%04d/dataList.txt", fSupplePath.Data(), fRunNo);
    metaFile = Form("%s/run_%04d/metadataList.txt", fSupplePath.Data(), fRunNo);
  }

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  STDecoderTask *decoder = new STDecoderTask();
  decoder -> SetUseSeparatedData(true);
  decoder -> SetPersistence(false);
  if (fGCData.IsNull())
    decoder -> SetUseGainCalibration(false);
  else {
    decoder -> SetUseGainCalibration(true);
    decoder -> SetGainCalibrationData(fGCData);
  }
  decoder -> SetGGNoiseData(fGGData);
  decoder -> SetDataList(raw);
  decoder -> SetEventID(start);

  if (fUseMeta) {
    std::ifstream metalistFile(metaFile.Data());
    TString dataFileWithPath;
    for (Int_t iCobo = 0; iCobo < 12; iCobo++) {
      dataFileWithPath.ReadLine(metalistFile);
      dataFileWithPath = Form("%s/run_%04d/%s", fSupplePath.Data(), fRunNo, dataFileWithPath.Data());
      decoder -> SetMetaData(dataFileWithPath, iCobo);
    }
  }

  auto preview = new STEventPreviewTask();
  preview -> SetSkippingEvents(fSkipEventArray);
  preview -> SetPersistence(true);

  auto psa = new STPSAGradTask();
  psa -> SetPersistence(true);
  psa -> SetThreshold(fPSAThreshold);
  psa -> SetLayerCut(-1, 90);

  auto helix = new STHelixTrackingTask();
  helix -> SetPersistence(true);
  helix -> SetClusterPersistence(false);
  helix -> SetClusteringOption(2);
  helix -> SetSaturationOption(1); 
  
  auto genfitPID = new STGenfitPIDTask();
  genfitPID -> SetPersistence(true);
  genfitPID -> SetBDCFile("");  
  //  genfitPID -> SetBDCFile("beam_run2894.ridf.root");
  genfitPID -> SetConstantField();
  
  run -> AddTask(decoder);
  run -> AddTask(preview);
  run -> AddTask(psa);
  run -> AddTask(helix);
  run -> AddTask(genfitPID);

  auto outFile = FairRootManager::Instance() -> GetOutFile();
  auto recoHeader = new STRecoHeader("RecoHeader","");
  recoHeader -> SetPar("version", version);
  recoHeader -> SetPar("eventStart", start);
  recoHeader -> SetPar("numEvents", fNumEventsInSplit);
  recoHeader -> SetPar("parameter", fParameterFile);
  recoHeader -> SetPar("GCData", fGCData);
  recoHeader -> SetPar("GGData", fGGData);
  recoHeader -> Write("RecoHeader");

  run -> Init();
  helix -> GetTrackFinder() -> SetDefaultCutScale(2.5);
  helix -> GetTrackFinder() -> SetTrackWidthCutLimits(4, 10);
  helix -> GetTrackFinder() -> SetTrackHeightCutLimits(2, 4);

  run -> Run(0, fNumEventsInSplit);

  cout << "Log    : " << log << endl;
  cout << "Input  : " << raw << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();
}
