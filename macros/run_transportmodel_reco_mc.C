void run_transportmodel_reco_mc
(
  TString fName = "urqmd_short",
  TString fPathToOutput = "",
  TString fPathToInput = "",
  Double_t fPSAThreshold = 30,
  TString fParameterFile = "ST.parameters.PhysicsRuns_201707.par"
)
{

  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToInput.IsNull())
    fPathToInput = spiritroot+"macros/data/";
  if (fPathToOutput.IsNull())
    fPathToOutput = spiritroot+"macros/data/";
  TString version; {
    TString name = spiritroot + "VERSION.compiled";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }
  TString par = spiritroot+"parameters/"+fParameterFile;
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root";
  TString in1 = fPathToInput+fName+".digi.root";
  TString in2 = fPathToInput+fName+".mc.root";
  TString out = fPathToOutput+fName+".reco."+version+".root";
  TString log = fPathToOutput+fName+"."+version+".log";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile(in1);
  run -> AddFriend(in2);
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto preview = new STEventPreviewTask();
  preview -> SetPersistence(true);

  auto mcevent = new STMCEventTask();
  mcevent -> SetPersistence(true);

  auto psa = new STPSAETask();
  psa -> SetPersistence(false);
  psa -> SetThreshold(fPSAThreshold);
  psa -> SetLayerCut(-1, 112);
  psa -> SetPulserData("pulser_117ns_50tb.dat");
  psa -> SetPSAPeakFindingOption(1);

  auto helix = new STHelixTrackingTask();
  helix -> SetPersistence(false);
  helix -> SetClusterPersistence(false);
  helix -> SetClusteringOption(2);
  helix -> SetSaturationOption(1);
  helix -> SetClusterCutLRTB(420,-420,-64,-522);
  
  auto genfitPID = new STGenfitPIDTask();
  genfitPID -> SetPersistence(true);
  genfitPID -> SetBDCFile("");
  //genfitPID -> SetConstantField();
  genfitPID -> SetListPersistence(true);

  auto genfitVA = new STGenfitVATask();
  genfitVA -> SetPersistence(true);
  //genfitVA -> SetConstantField();
  genfitVA -> SetListPersistence(true);

  run -> AddTask(preview);
  run -> AddTask(mcevent);
  run -> AddTask(psa);
  run -> AddTask(helix);
  run -> AddTask(genfitPID);
  //run -> AddTask(genfitVA);

  auto outFile = FairRootManager::Instance() -> GetOutFile();
  auto recoHeader = new STRecoHeader("RecoHeader","");
  recoHeader -> SetPar("version", version);
  recoHeader -> SetPar("parameter", fParameterFile);
  recoHeader -> Write("RecoHeader");

  run -> Init();
  helix -> GetTrackFinder() -> SetDefaultCutScale(2.5);
  helix -> GetTrackFinder() -> SetTrackWidthCutLimits(4, 10);
  helix -> GetTrackFinder() -> SetTrackHeightCutLimits(2, 4);
  run -> Run(0, 0);

  cout << "Log    : " << log << endl;
  cout << "Input1 : " << in1 << endl;
  cout << "Input2 : " << in2 << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();
}
