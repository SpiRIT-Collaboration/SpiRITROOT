void run_reco_hokusai
(
  TString fName = "test",

  Int_t fRunNo = 2894,
  Int_t fNumEventsInRun = 100,
  Int_t fSplitNo = 0,
  Int_t fNumEventsInSplit = 200,

  TString fGCData = "",
  TString fGGData = "/data/Q18393/production/ggNoise/ggNoise_2895.root",

  Bool_t fClusterCov = false,
  Bool_t fYOffset = false,

  Bool_t fHitSep = false,
  Bool_t fTwoHitSep = false,

  TString fYOffsetCalibration = "/home/ejungwoo/macros/yoffset/data/yoffset_calibration_layer_row.dat",
  TString fBDC = "/data/Q18393/common/ridf",
  std::vector<Int_t> fSkipEventArray = {},

  Double_t fPSAThreshold = 30,
   TString fParameterFile = "ST.parameters.PhysicsRuns_201707.par",
   TString fPathToData = "",
    Bool_t fUseMeta = true,
   TString fSupplePath = "/data/Q18393/rawdataSupplement",
    Bool_t fUseFieldMap = true,

  Bool_t fSaveDecoder = false,
  Bool_t fSaveHit = false,
  Bool_t fSaveHelix = true,
  Bool_t fSaveCluster = true,
  Bool_t fSaveRecoTrack = true, //
  Bool_t fSaveRecoTrackList = false, //
  Bool_t fSaveVaddTrack = true, //
  Bool_t fSaveVaddTrackList = false, //

  TString fEmbedFile = ""
)
{
  fName = "test";

  cout << "[run_reco_hokusai]" << endl;
  if (fPathToData.IsNull()) {
    fPathToData = Form("/data/Q18393/recodata/run%d/",fRunNo);
    gSystem -> Exec(TString("mkdir -p ")+fPathToData);
  }

  Int_t start = fSplitNo * fNumEventsInSplit;
  if (start >= fNumEventsInRun) {
    cout << "Number of events in RUN_" << fRunNo << " is " << fNumEventsInRun << " but run start from " << start << endl;
    return;
  }
  if (start + fNumEventsInSplit > fNumEventsInRun)
    fNumEventsInSplit = fNumEventsInRun - start;

  cout << "Start No.: " << start << endl;

  TString sRunNo   = TString::Itoa(fRunNo, 10);
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToData.IsNull())
    fPathToData = spiritroot+"macros/data/";
  TString version; {
    TString name = spiritroot + "VERSION.compiled";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  Bool_t digi = false;
  TString input;
  TString out;
  TString log;

  if (!fName.IsNull()) {
    if (fName.Index(".digi.")>0) {
      input = fName;
      out = input;
      log = input;
      out.ReplaceAll(".digi.",".reco.");
      log.ReplaceAll(".digi.",".log.");
      digi = true;
    }
    else
      version = version + "." + fName;
  }

  TString par = spiritroot+"parameters/"+fParameterFile;
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root";
  TString raw = TString(gSystem -> Getenv("PWD"))+"/list_run"+sRunNo+".txt";
          out = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".reco."+version+".root";
          log = fPathToData+"run"+sRunNo+"_s"+sSplitNo+"."+version+".log";

  TString yoffset = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".yoffset."+version+".root";
  TString cluster_cov = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".cluster_cov."+version+".root";
  TString hit_sep = fPathToData+"run"+sRunNo+"_s"+sSplitNo+".hit_sep."+version+".root";

  Bool_t embeddingTest = false;
  if (!fEmbedFile.IsNull())
    embeddingTest  = true;

  if (TString(gSystem -> Which(".", raw)).IsNull() && !fUseMeta)
    gSystem -> Exec("./createList.sh "+sRunNo);

  TString metaFile;
  if (fUseMeta) {
    raw = Form("%s/run_%04d/dataList.txt", fSupplePath.Data(), fRunNo);
    metaFile = Form("%s/run_%04d/metadataList.txt", fSupplePath.Data(), fRunNo);
  }

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);
  logger -> SetLogToFile(true);
  logger -> SetLogFileName(log);
  logger -> SetLogVerbosityLevel("LOW");

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  if (digi)
    run -> SetInputFile(input);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  STDecoderTask *decoder = new STDecoderTask();
  if (!digi) {
    decoder -> SetUseSeparatedData(true);
    decoder -> SetPersistence(fSaveDecoder);
    decoder -> SetUseGainCalibration(true);
    if (!fGCData.IsNull())
      decoder -> SetGainCalibrationData(fGCData);
    //else  automatically added from SpiRITROOT/paramameter/
    decoder -> SetGGNoiseData(fGGData);
    decoder -> SetDataList(raw);
    decoder -> SetEventID(start);
    if (embeddingTest) {
      decoder -> SetEmbedding(embeddingTest);
      decoder -> SetEmbedFile(fEmbedFile);
    }

    if (fUseMeta) {
      std::ifstream metalistFile(metaFile.Data());
      TString dataFileWithPath;
      for (Int_t iCobo = 0; iCobo < 12; iCobo++) {
        dataFileWithPath.ReadLine(metalistFile);
        dataFileWithPath = Form("%s/run_%04d/%s", fSupplePath.Data(), fRunNo, dataFileWithPath.Data());
        decoder -> SetMetaData(dataFileWithPath, iCobo);
      }
    }
  }

  auto preview = new STEventPreviewTask();
  preview -> SetSkippingEvents(fSkipEventArray);
  preview -> SetPersistence(true);

  auto psa = new STPSAETask();
  psa -> SetPersistence(fSaveHit);
  psa -> SetThreshold(fPSAThreshold);
  psa -> SetLayerCut(-1, 112);
  psa -> SetEmbedding(embeddingTest);
  psa -> SetPulserData("pulser_117ns_50tb.dat");
  psa -> SetPSAPeakFindingOption(1);
  if (!fYOffsetCalibration.IsNull())
    psa -> SetYOffsets(fYOffsetCalibration);

  auto helix = new STHelixTrackingTask();
  helix -> SetPersistence(fSaveHelix);
  helix -> SetClusterPersistence(fSaveCluster);
  helix -> SetClusteringOption(2);
  //helix -> SetSaturationOption(1); 
  helix -> SetClusterCutLRTB(420, -420, -64, -522);

  auto corr = new STCorrectionTask();
  
  auto genfitPID = new STGenfitPIDTask();
  genfitPID -> SetPersistence(fSaveRecoTrack);
  genfitPID -> SetBDCFile("");  
  if (!fUseFieldMap)
    genfitPID -> SetConstantField();
  genfitPID -> SetListPersistence(fSaveRecoTrackList);
  //genfitPID -> SetPadHitPersistence(fSavePadHit);

  TString bdcName = Form("%s/beam_run%d.ridf.root",fBDC.Data(),fRunNo);
  if (!fBDC.IsNull())
    genfitPID -> SetBDCFile(bdcName);

  //if (fYOffset)    genfitPID -> MakeYOffsetCalibrationFile(yoffset);
  //if (fClusterCov) genfitPID -> MakeClusterCovFile(cluster_cov);

  auto genfitVA = new STGenfitVATask();
  genfitVA -> SetPersistence(fSaveVaddTrack);
  if (!fUseFieldMap)
    genfitVA -> SetConstantField();
  genfitVA -> SetListPersistence(fSaveVaddTrackList);
  if (!fBDC.IsNull())
    genfitVA -> SetBeamFile(bdcName);
  genfitVA -> SetInformationForBDC(fRunNo, /* xOffset */ -0.507, /* yOffset */ -227.013);
  
  auto embedCorr = new STEmbedCorrelatorTask();
  embedCorr -> SetPersistence(embeddingTest);

  //auto hs = new STHitSeparationTask(fTwoHitSep);
  //hs -> SetHitSeparationFile(hit_sep);

  if (!digi)
    run -> AddTask(decoder);
  run -> AddTask(preview);
  run -> AddTask(psa);
  run -> AddTask(helix);
  run -> AddTask(corr);
  run -> AddTask(genfitPID);
  //run -> AddTask(genfitVA);
  if (embeddingTest)
    run -> AddTask(embedCorr);
  //if (fHitSep) run -> AddTask(hs);

  auto outFile = FairRootManager::Instance() -> GetOutFile();
  auto recoHeader = new STRecoHeader("RecoHeader","");
  recoHeader -> SetPar("version", version);
  recoHeader -> SetPar("run",fRunNo);
  recoHeader -> SetPar("numEventsInRun",fNumEventsInRun);
  recoHeader -> SetPar("split",fSplitNo);
  recoHeader -> SetPar("eventStart", start);
  recoHeader -> SetPar("numEvents", fNumEventsInSplit);
  recoHeader -> SetPar("parameter", fParameterFile);
  if (!fGCData.IsNull())
    recoHeader -> SetPar("GCData", fGCData);
  recoHeader -> SetPar("GGData", fGGData);
  recoHeader -> Write("RecoHeader");

  run -> Init();
  //helix -> GetTrackFinder() -> SetDefaultCutScale(2.5);
  //helix -> GetTrackFinder() -> SetTrackWidthCutLimits(4, 10);
  //helix -> GetTrackFinder() -> SetTrackHeightCutLimits(2, 4);

  run -> Run(0, fNumEventsInSplit);
  //if (fYOffset)    genfitPID -> WriteYOffsetCalibrationFile();
  //if (fClusterCov) genfitPID -> WriteClusterCovFile();
  //if (fHitSep)     hs -> WriteHitSeparationFile();

  cout << "Tag    : " << version << endl;
  cout << "Log    : " << log << endl;
  cout << "Input  : " << raw << endl;
  cout << "Output : " << out << endl;

  //if (fYOffset)    cout << "y-offset          : " << yoffset << endl;
  //if (fClusterCov) cout << "cluster-covariance: " << cluster_cov << endl;
  //if (fHitSep)     cout << "hit-separation    : " << hit_sep << endl;

  gApplication -> Terminate();
}
