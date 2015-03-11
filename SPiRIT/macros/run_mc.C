void run_mc(TString tag = "test")
{
  gRandom -> SetSeed(time(0));
  gDebug = 0;

  // -- Inviroment Setting -------------------------------------------------
  TString workDir = gSystem->Getenv("SPIRITDIR");
  TString geoDir  = workDir + "/geometry";
  TString confDir = workDir + "/gconfig";
  TString outFile = "data/spirit_" + tag + ".mc.root"; 
  TString parFile = "data/spirit_" + tag + ".params.root"; 
  gSystem -> Setenv("GEOMPATH",   geoDir.Data());
  gSystem -> Setenv("CONFIG_DIR", confDir.Data());


  // -----   Logger  --------------------------------------------------------
  FairLogger *logger = FairLogger::GetLogger();
  logger->SetLogFileName("data/LOG_"+tag+".log"); // define log file name
  logger->SetLogToScreen(kTRUE); // log to screen and to file 
  logger->SetLogToFile(kTRUE);
  logger->SetLogVerbosityLevel("HIGH"); //(LOW, MEDIUM, HIGH)


  // -----   Create simulation run   ----------------------------------------
  FairRunSim* run = new FairRunSim();


  // -----   Create geometry   ----------------------------------------------
  FairModule* cave= new FairCave("CAVE");
  cave -> SetGeometryFileName("cave_vacuum.geo"); 
  FairDetector* spirit = new STDetector("STDetector", kTRUE);
  spirit -> SetGeometryFileName("geomSPiRIT.root");


  // -----   Create and set magnetic field   --------------------------------
  FairConstField *fMagField = new FairConstField();
  fMagField -> SetField(0., 5., 0.); // in kG
  fMagField -> SetFieldRegion(-90.275,90.2752,-95.55/2,95.55/2,-104.82/2,104.82/2);

  /** 
   * Use field map 
   * Samurai Field Map implimentation, 
   * SamuraiMap_0.5T.dat file should be placed in the "input" directory
   **/
  //STFieldMap *fMagField = new STFieldMap("SamuraiMap_0.5T","A");
  //fMagField->SetPosition(-130.55/2,-51.10/2,-159.64/2);


  // -----   Create PrimaryGenerator   --------------------------------------
  STEventGenGenerator* gen = new STEventGenGenerator("UrQMD_300AMeV_short.egen");
  gen->SetPrimaryVertex(0,-21,-3);
  Int_t nEvents = gen->GetNEvents();
  cout << "Number of events : " << nEvents << endl;
  FairPrimaryGenerator *primGen = new FairPrimaryGenerator();
  primGen->AddGenerator(gen);


  // -----   Run initialisation   -------------------------------------------
  run -> SetName("TGeant4");              // Transport engine
  run -> SetOutputFile(outFile);          // Output file
  run -> SetWriteRunInfoFile(kFALSE);  
  run -> SetMaterials("media.geo");      
  run -> AddModule(cave);
  run -> AddModule(spirit);
  run -> SetField(fMagField);
  run -> SetGenerator(primGen);  
  run -> SetStoreTraj(kTRUE);
  run -> Init();
 

  /**
   * Set cuts for storing the trajectories.
   * Switch this on only if trajectories are stored.
   * Choose this cuts according to your needs, but be aware
   * that the file size of the output file depends on these cuts
   *
   * Ok so if we see the line four lines above this it says 
   * Switch this on only if trajectories are stored. 
   * What that means is that there actually must be a 
   * FairTrajFilter in order to store them. 
   * It does not appear to need the SetStorePrimaries 
   * or SetStoreSecondaries to NEED to be true 
   * but I'm leaving those lines uncommented so as to make sure.
   *
   **/
  FairTrajFilter* trajFilter = FairTrajFilter::Instance();
  trajFilter -> SetStorePrimaries(kTRUE);
  trajFilter -> SetStoreSecondaries(kTRUE);


  // -----   Runtime database   ---------------------------------------------
  Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
                     parOut -> open(parFile.Data());

  FairRuntimeDb* rtdb = run -> GetRuntimeDb();
                 rtdb -> setOutput(parOut);
                 rtdb -> saveOutput();
                 rtdb -> print();


  // -----   Start run   ----------------------------------------------------
  run -> Run(nEvents);
  delete run;
}
