void run_reco_experiment
(
 Int_t fRunNo = 2894,
 Int_t fNumEventsInRun = 25000,
 Int_t fSplitNo = 0,
 Int_t fNumEventsInSplit = 10,
 TString fGCData = "",
 TString fGGData = "/mnt/spirit/rawdata/misc/ggNoise/ggNoise_2886.root",
 std::vector<Int_t> fSkipEventArray = {},
 Double_t fPSAThreshold = 30,
 TString fParameterFile = "ST.parameters.PhysicsRuns_201707.par",
 TString fPathToData = "",
 Bool_t fUseMeta = kTRUE,
 TString EmbeddedTag = "",//EvtProton",
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
  { fPathToData = spiritroot+"macros_PulseShapeAna/data/"; }
  TString version;
  {
    TString name = spiritroot + "VERSION.compiled";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }
  TString par = spiritroot+"parameters/"+fParameterFile;
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root";
  TString raw = TString(gSystem -> Getenv("PWD"))+"/list_run"+sRunNo+".txt";
  TString out = fPathToData+ EmbeddedTag +"_run"+sRunNo+"_s"+sSplitNo+"_10Evts_PSA_PeakFinding_2.reco"+".root";
  TString log = fPathToData+ EmbeddedTag +"_run"+sRunNo+"_s"+sSplitNo+".log";
  
  if (TString(gSystem -> Which(".", raw)).IsNull() && !fUseMeta)
    gSystem -> Exec("./createList.sh "+sRunNo);
  
  TString metaFile;
  if (fUseMeta)
  {
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
  decoder -> SetPersistence(0);
//  if (fGCData.IsNull())
//  { decoder -> SetUseGainCalibration(false); }
//  else 
  {
    decoder -> SetUseGainCalibration(true);
    //decoder -> SetGainCalibrationData(fGCData); // automatically added from param.
  }
  decoder -> SetGGNoiseData(fGGData);
  decoder -> SetDataList(raw);
  decoder -> SetEventID(start);
  decoder -> SetEmbedding(0);
  decoder -> SetEmbedFile("");//./data/Run2894_Total_ProtonTrackContainer.root");
  //");//./data/_run3209_s0.reco.develop.1543.3db6f38.root");//"");//"./data/Run2894_OnTarget_PionTrackContainer.root");
  //"./data/one_proton_0.6GeV.digi.root"); //"./data/TrackContainer.root");
  
  if (fUseMeta)
  {
    std::ifstream metalistFile(metaFile.Data());
    TString dataFileWithPath;
    for (Int_t iCobo = 0; iCobo < 12; iCobo++)
    {
      dataFileWithPath.ReadLine(metalistFile);
      dataFileWithPath = Form("%s/run_%04d/%s", fSupplePath.Data(), fRunNo, dataFileWithPath.Data());
      decoder -> SetMetaData(dataFileWithPath, iCobo);
    }
  }
  
  auto preview = new STEventPreviewTask();
  preview -> SetSkippingEvents(fSkipEventArray);
  preview -> SetPersistence(0);
  
  auto psa = new STPSAETask();
  psa -> SetPersistence(1);
  psa -> SetThreshold(fPSAThreshold);
  psa -> SetEmbedding(0);
  psa -> SetLayerCut(-1, 90);
  psa -> SetPulserData("pulser_117ns.dat");
  psa -> Set_PSA_PeakFinding_Opt(1); //1: JungWoo's original one, 2: high efficiency for small pulse.
  
  auto helix = new STHelixTrackingTask();
  helix -> SetPersistence(1);
  helix -> SetClusterPersistence(1);
  helix -> SetClusteringOption(2);
  helix -> SetSaturationOption(1);
  
  auto genfitPID = new STGenfitPIDTask();
  genfitPID -> SetPersistence(true);
  genfitPID -> SetBDCFile("");
  //genfitPID -> SetConstantField();
  genfitPID -> SetListPersistence(true);
  
  auto genfitVA = new STGenfitVATask();
  genfitVA -> SetPersistence(true);
  genfitVA -> SetConstantField();
  genfitVA -> SetListPersistence(true);
//  genfitVA -> SetBeamFile("");
  genfitVA -> SetBeamFile(Form("/mnt/spirit/analysis/changj/BeamAnalysis/macros/output/beam.Sn132_all/beam_run%d.ridf.root", fRunNo));
  genfitVA -> SetInformationForBDC(fRunNo, /* xOffset */ -0.507, /* yOffset */ -227.013);
  
  auto embedCorr = new STEmbedCorrelatorTask();
  embedCorr -> SetPersistence(0);
/*  
  //task of collecting the tracks
  const int ParticleNum = 1; int ParticleID[] = {1};
  STPickTrackTask* TrackPicker = new STPickTrackTask();
  TrackPicker->SetRunNo(fRunNo);
  TrackPicker->SetParticleCollected(ParticleNum, ParticleID);
  TrackPicker->SetStoreFileName("./data/Run2894_OnTarget_Pion_STDPulse_500Evts_PSA_NewPeakFinding_1.root");
  TrackPicker->Set_LimitBoundary(2,0.1,0.04); // the position, Magnitude percentage, angle different;
  TrackPicker->SetPIDProbability(0.5); // get a stable track;
  TrackPicker->SetEventTrackNum(10, 80); // the cocktail will only choose 1 track/event, so this function only used fot the event.
  TrackPicker->SetMinCluster(30);
  TrackPicker->SetTrackCut_VertexDistance(10);
  TrackPicker->SetIsPickTrackCutUsed(1); //Now the cut only include the pion.
  TrackPicker->SetIsStoreEachEvent(0);
  TrackPicker->SetIsRawData_Recorded(0);
  TrackPicker->SetEvtTag("event","OnTarget"); //(event, cocktail),(OnTarget,Total) (ActiveTarget is not considered now!)
*/
  run -> AddTask(decoder);
  run -> AddTask(preview);
  run -> AddTask(psa);
  run -> AddTask(helix);
  run -> AddTask(genfitPID);
  run -> AddTask(genfitVA);
//  run -> AddTask(embedCorr);
//  run -> AddTask(TrackPicker);
  
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
//  TrackPicker->GetEmbedContainer()->SaveData();

  cout << "Log    : " << log << endl;
  cout << "Input  : " << raw << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();
}
