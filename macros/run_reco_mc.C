void run_reco_mc
(
  TString fName = "proton1000",
  Int_t fRunNo = 0,
  Int_t fNumEventsInRun = 2500,
  Int_t fSplitNo = 0,
  Int_t fNumEventsInSplit = 2500,
  Double_t fPSAThreshold = 30,
  TString fParameterFile = "ST.parameters.Commissioning_201604.par",
  TString fPathToData = ""
)
{
  Int_t clusteringOption = 2,

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
  TString mc  = fPathToData+fName+".mc.root";
  TString out = fPathToData+fName+".reco."+version+".root";
  TString log = fPathToData+fName+"."+version+".log";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> AddFriend(mc);
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto preview = new STEventPreviewTask();
  preview -> SetPersistence(true);

  auto psa = new STPSAETask();
  psa -> SetPersistence(false);
  psa -> SetThreshold(fPSAThreshold);
  psa -> SetLayerCut(-1, 112);
  psa -> SetPulserData("pulser_117ns.dat");

  auto helix = new STHelixTrackingTask();
  helix -> SetPersistence(true);
  helix -> SetClusterPersistence(true);
  helix -> SetClusteringOption(clusteringOption);

  auto st_genfit = new STGenfitETask();
  st_genfit -> SetPersistence(true);
  st_genfit -> SetClusteringType(clusteringOption);

  auto mctruth = new STMCTruthTask(true);

  run -> AddTask(preview);
  run -> AddTask(psa);
  run -> AddTask(helix);
  run -> AddTask(st_genfit);
  run -> AddTask(mctruth);

  auto outFile = FairRootManager::Instance() -> GetOutFile();
  auto recoHeader = new STRecoHeader("RecoHeader","");
  recoHeader -> SetPar("version", version);
  recoHeader -> SetPar("eventStart", start);
  recoHeader -> SetPar("numEvents", fNumEventsInSplit);
  recoHeader -> SetPar("parameter", fParameterFile);
  recoHeader -> Write("RecoHeader");

  run -> Init();
  run -> Run(0, fNumEventsInSplit);

  //st_genfit -> OpenDisplay();

  cout << "Log    : " << log << endl;
  cout << "MC     : " << mc  << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();
}
