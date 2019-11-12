void run_reco_mc
(
  TString fName = "he4_lowmom",
  TString fOutName = "",
  //Int_t fRunNo = 0,
  Int_t fNumEventsInRun = 10000,
  Int_t fSplitNo = 0,
  Int_t fNumEventsInSplit = 10000,
  Double_t fPSAThreshold = 30,
  TString fParameterFile = "ST.parameters.fullmc.par",
  TString fMCFile = "",
  TString fPathToData = ""
)
{
  Int_t start = fSplitNo * fNumEventsInSplit;
  if (start >= fNumEventsInRun) return;
  if (start + fNumEventsInSplit > fNumEventsInRun)
    fNumEventsInSplit = fNumEventsInRun - start;

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

  if(fOutName.IsNull()) fOutName = fName;

  TString par = spiritroot+"parameters/"+fParameterFile;
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root";
  TString in1 = fPathToData+fName+".digi.root"; 
  TString in2 = fPathToData+fName+".mc.root"; 
  TString out = fPathToData+fOutName+"_s"+sSplitNo+".reco."+version+".root";
  TString log = fPathToData+fOutName+"_s"+sSplitNo+"."+version+".log";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairFileSource *inputFile = nullptr;
  // use TCHain to search for all files
  TChain chain("cbmsim");
  chain.Add(in1);
  auto fileList = chain.GetListOfFiles();
  int nfiles = fileList->GetEntries();
  inputFile = new FairFileSource(fileList->At(0)->GetTitle());
  for(int i = 1; i < nfiles; ++i) 
    inputFile->AddFile(fileList->At(i)->GetTitle());


  FairRunAna* run = new FairRunAna();
  run -> SetSource(inputFile);
  //run -> AddFriend(in2);
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto embedTask = new STEmbedTask();
  embedTask -> SetEventID(start);
  embedTask -> SetEmbedFile(fMCFile);

  auto preview = new STEventPreviewTask();
  preview -> SetPersistence(false);

  auto psa = new STPSAETask();
  psa -> SetPersistence(false);
  psa -> SetThreshold(fPSAThreshold);
  psa -> SetLayerCut(-1, 112);
  psa -> SetPulserData("pulser_117ns_50tb.dat");
  psa -> SetPSAPeakFindingOption(1);
  psa -> SetGainMatchingData(spiritroot + "parameters/RelativeGain.list");
  psa -> SetYOffsets(spiritroot + "parameters/yOffsetCalibration.dat");
  
  auto helix = new STHelixTrackingTask();
  helix -> SetPersistence(false);
  helix -> SetClusterPersistence(true);
  double YPedestalOffset = 21.88;
  helix -> SetClusterCutLRTB(420, -420, -64+YPedestalOffset, -522+YPedestalOffset);
  helix -> SetEllipsoidCut(TVector3(0, -260+YPedestalOffset, -11.9084), TVector3(120, 100, 220), 5); // current us

  auto correct = new STCorrectionTask(); //Correct for saturation   

  auto spaceCharge = new STSpaceChargeCorrectionTask();
  auto gfBField = STGFBField::GetInstance("samurai_field_map", "A", -0.1794, -20.5502, 58.0526);   
  spaceCharge -> SetBField(gfBField -> GetFieldMap());
  spaceCharge -> SetSheetChargeDensity(4e-8);
  spaceCharge -> SetProjectile(STSpaceCharge::Projectile::Sn132);
  spaceCharge -> SetElectronDrift(true); 
  
  auto genfitPID = new STGenfitPIDTask();
  genfitPID -> SetFieldOffset(-0.1794, -20.5502, 58.0526); //unit: cm, which comes from Jon's measurement. It means the position of magnetic field in the TPC frame.
  genfitPID -> SetTargetPlane(0,-203.3,-13.2); // unit: mm, in the TPC frame. here the z position is used when Genfit do the extrapolation.
  genfitPID -> SetPersistence(true);
  genfitPID -> SetBDCFile("");
  genfitPID -> SetPersistence(true);

  auto genfitVA = new STGenfitVATask();
  genfitVA -> SetPersistence(true);
  genfitVA -> SetFixedVertex(0.,-203.3,-13.2);
  genfitVA -> SetUseRave(true);
  genfitVA -> SetFieldOffset(-0.1794, -20.5502, 58.0526); 

  auto embedCorr = new STEmbedCorrelatorTask();
  embedCorr -> SetPersistence(true);
  TString out2 = fPathToData+fOutName+"_s"+sSplitNo+".reco."+version+".conc.root";
  auto smallOutput = new STSmallOutputTask();
  smallOutput -> SetOutputFile(out2.Data());

  auto mctruth = new STMCTruthTask(true);

  if(!fMCFile.IsNull())
    run -> AddTask(embedTask);
  run -> AddTask(preview);
  run -> AddTask(psa);
  run -> AddTask(helix);
  run -> AddTask(correct);
  run -> AddTask(spaceCharge);
  run -> AddTask(genfitPID);
  run -> AddTask(genfitVA);
  if(!fMCFile.IsNull())
    run -> AddTask(embedCorr);
  run -> AddTask(smallOutput);
  //run -> AddTask(mctruth);

  auto outFile = FairRootManager::Instance() -> GetOutFile();
  auto recoHeader = new STRecoHeader("RecoHeader","");
  recoHeader -> SetPar("version", version);
  recoHeader -> SetPar("eventStart", start);
  recoHeader -> SetPar("numEvents", fNumEventsInSplit);
  recoHeader -> SetPar("parameter", fParameterFile);
  recoHeader -> Write("RecoHeader");

  run -> Init();
  run -> Run(start, start + fNumEventsInSplit);
  //run -> Run(0, 2);

  cout << "Log    : " << log << endl;
  cout << "Input1 : " << in1 << endl;
  cout << "Input2 : " << in2 << endl;
  cout << "Output : " << out << endl;

  delete correct;
  gApplication -> Terminate();

}
