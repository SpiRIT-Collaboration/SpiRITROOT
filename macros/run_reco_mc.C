void run_reco_mc
(
  TString fName = "he4_lowmom",
  Int_t fRunNo = 0,
  Int_t fNumEventsInRun = 10000,
  Int_t fSplitNo = 0,
  Int_t fNumEventsInSplit = 10000,
  Double_t fPSAThreshold = 30,
  TString fParameterFile = "ST.parameters.fullmc.par",
  TString fPathToData = ""
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
  TString in1 = fPathToData+fName+".digi.root"; 
  TString in2 = fPathToData+fName+".mc.root"; 
  TString out = fPathToData+fName+"_s"+sSplitNo+".reco."+version+".root";
  TString log = fPathToData+fName+"_s"+sSplitNo+"."+version+".log";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  auto inputFile = new FairFileSource(in1);

  FairRunAna* run = new FairRunAna();
  run -> SetSource(inputFile);
  run -> AddFriend(in2);
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto preview = new STEventPreviewTask();
  preview -> SetPersistence(true);

  auto psa = new STPSAETask();
  psa -> SetPersistence(true);
  psa -> SetThreshold(fPSAThreshold);
  psa -> SetLayerCut(-1, 112);
  psa -> SetPulserData("pulser_117ns_50tb.dat");
  psa -> SetPSAPeakFindingOption(1);
  psa -> SetGainMatchingData(spiritroot + "parameters/RelativeGain.list");
  psa -> SetYOffsets(spiritroot + "parameters/yOffsetCalibration.dat");
  
  auto helix = new STHelixTrackingTask();
  helix -> SetPersistence(true);
  helix -> SetClusterPersistence(true);
  helix -> SetClusterCutLRTB(420, -420, -64, -522);
  helix -> SetEllipsoidCut(TVector3(0, -260, -11.9084), TVector3(60, 50, 110), 5); // current use
  

  auto correct = new STCorrectionTask(); //Correct for saturation   

  auto spaceCharge = new STSpaceChargeCorrectionTask();
  auto gfBField = STGFBField::GetInstance("samurai_field_map", "A", -0.1794, -20.5502, 58.0526);   
  spaceCharge -> SetBField(gfBField -> GetFieldMap());
  spaceCharge -> SetSheetChargeDensity(4e-8);
  spaceCharge -> SetProjectile(STSpaceCharge::Projectile::Sn132);
  spaceCharge -> SetElectronDrift(true); 
  
  auto genfitPID = new STGenfitPIDTask();
  genfitPID -> SetPersistence(true);

  auto genfitVA = new STGenfitVATask();
  genfitVA -> SetPersistence(true);
  genfitVA -> SetFixedVertex(0.,-213.3,-13.2);
  genfitVA -> SetUseRave(true);
  genfitVA -> SetFieldOffset(-0.1794, -20.5502, 58.0526); 
  
  auto mctruth = new STMCTruthTask(true);

  run -> AddTask(preview);
  run -> AddTask(psa);
  run -> AddTask(helix);
  run -> AddTask(correct);
  run -> AddTask(spaceCharge);
  run -> AddTask(genfitPID);
  run -> AddTask(genfitVA);
  run -> AddTask(mctruth);

  auto outFile = FairRootManager::Instance() -> GetOutFile();
  auto recoHeader = new STRecoHeader("RecoHeader","");
  recoHeader -> SetPar("version", version);
  recoHeader -> SetPar("eventStart", start);
  recoHeader -> SetPar("numEvents", fNumEventsInSplit);
  recoHeader -> SetPar("parameter", fParameterFile);
  recoHeader -> Write("RecoHeader");

  run -> Init();

  auto index = 1;
  while (kTRUE) {
    auto copyLine = in1;
    auto path = copyLine.ReplaceAll("digi", Form("digi_%d", index++));
    cout << path << endl;

    if (!(gSystem -> IsFileInIncludePath(path)))
      break;

    inputFile -> GetInChain() -> AddFile(path);
  }

  run -> Run(0, fNumEventsInSplit);
  //run -> Run(0, 2);

  cout << "Log    : " << log << endl;
  cout << "Input1 : " << in1 << endl;
  cout << "Input2 : " << in2 << endl;
  cout << "Output : " << out << endl;

  delete correct;
  gApplication -> Terminate();

}
