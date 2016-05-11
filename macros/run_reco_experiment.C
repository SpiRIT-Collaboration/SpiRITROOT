void run_reco_experiment
(
  Int_t fRunNo = 2621,
  Int_t fNumEventsInRun = 5000,
  Int_t fSplitNo = 0,
  Int_t fNumEventsInSplit = 1000,
  TString fParameterFile = "ST.parameters.Commissioning_201604.par"
)
{
  Int_t start = fSplitNo * fNumEventsInSplit;
  if (start >= fNumEventsInRun) return;
  if (start + fNumEventsInSplit > fNumEventsInRun)
    fNumEventsInSplit = fNumEventsInRun - start;

  TString sRunNo     = TString::Itoa(fRunNo, 10);
  TString sSplitNo   = TString::Itoa(fSplitNo, 10);
  TString sNumEvents = TString::Itoa(fNumEventsInSplit, 10);

  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  TString pathToData = spiritroot+"macros/data/";
  TString par = spiritroot+"parameters/"+fParameterFile;
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root";
  TString raw = TString(gSystem -> Getenv("PWD"))+"/list_run"+sRunNo+".txt";
  TString out = pathToData+"run"+sRunNo+"_s"+sSplitNo+"_n"+sNumEvents+".reco.root"; 
  TString log = pathToData+"run"+sRunNo+"_s"+sSplitNo+"_n"+sNumEvents+".log"; 

  if (TString(gSystem -> Which(".", raw)).IsNull())
    gSystem -> Exec("./createList.sh "+sRunNo);

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogFileName(log);
  logger -> SetLogToScreen(true);
  logger -> SetLogToFile(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  STDecoderTask *decoder = new STDecoderTask();
  decoder -> SetUseSeparatedData(true);
  decoder -> SetPersistence(false);
  decoder -> SetUseGainCalibration(false);
  decoder -> SetGGNoiseData("");
  decoder -> SetDataList(raw);
  decoder -> SetEventID(start);

  STEventPreviewTask *preview = new STEventPreviewTask();
  preview -> SetPersistence(false);

  STPSAETask *psa = new STPSAETask();
  psa -> SetPersistence(false);
  psa -> SetThreshold(30);
  psa -> SetLayerCut(-1, 90);

  STCurveTrackingETask *curve = new STCurveTrackingETask();
  curve -> SetPersistence(false);

  STHitClusteringCTTask *clustering = new STHitClusteringCTTask();
  clustering -> SetPersistence(false);

  STSMETask *sm = new STSMETask();
  sm -> SetPersistence(false);

  STRiemannTrackingETask *riemann = new STRiemannTrackingETask();
  riemann -> SetPersistence(false);

  STGenfitETask *genfit = new STGenfitETask();
  genfit -> SetPersistence(true);
  genfit -> SetRemoveNoVertexEvent(false);

  run -> AddTask(decoder);
  run -> AddTask(preview);
  run -> AddTask(psa);
  run -> AddTask(curve);
  run -> AddTask(clustering);
  run -> AddTask(sm);
  run -> AddTask(riemann);
  run -> AddTask(genfit);

  run -> Init();
  run -> Run(0, fNumEventsInSplit);

  cout << "Log    : " << log << endl;
  cout << "Input  : " << raw << endl;
  cout << "Output : " << out << endl;
}
