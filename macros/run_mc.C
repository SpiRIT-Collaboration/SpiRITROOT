/**
 * Geant4 simulation macro
 */

void run_mc
(
  TString name  = "urqmd_short",
  TString event = "UrQMD_300AMeV_short.egen",
  Bool_t  useFieldMapFile = kFALSE
)
{
  //////////////////////////////////////////////////////////
  //                                                      //
  //   In general, the below parts need not be touched.   //
  //                                                      //
  //////////////////////////////////////////////////////////


  // -----------------------------------------------------------------
  // Set enveiroment
  TString workDir   = gSystem -> Getenv("VMCWORKDIR");
  TString dataDir   = workDir + "/macros/data/";
  TString geomDir   = workDir + "/geometry";
  TString g4ConfDir = workDir + "/gconfig";


  // -----------------------------------------------------------------
  // Set file names
  TString outputFile = dataDir + name + ".mc.root"; 
  TString outParFile = dataDir + name + ".params.root";
  TString loggerFile = dataDir + "log_" + name + ".mc.txt";


  // -----------------------------------------------------------------
  // MC initialization
  gRandom -> SetSeed(time(0));
  gSystem -> Setenv("GEOMPATH",   geomDir.Data());
  gSystem -> Setenv("CONFIG_DIR", g4ConfDir.Data());


  // -----------------------------------------------------------------
  // Set FairRunSim
  FairRunSim* fRun = new FairRunSim();
  fRun -> SetName("TGeant4");
  fRun -> SetOutputFile(outputFile);
  fRun -> SetWriteRunInfoFile(kFALSE);
  fRun -> SetMaterials("media.geo");


  // -----------------------------------------------------------------
  // Logger
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger -> SetLogFileName(loggerFile); 
  fLogger -> SetLogToScreen(kTRUE); 
  fLogger -> SetLogToFile(kTRUE);
  fLogger -> SetLogVerbosityLevel("LOW");


  // -----------------------------------------------------------------
  // Geometry
  FairModule* fCave= new FairCave("CAVE");
  fCave -> SetGeometryFileName("cave_vacuum.geo"); 
  fRun -> AddModule(fCave);

  FairDetector* fSpiRIT = new STDetector("STDetector", kTRUE);
  fSpiRIT -> SetGeometryFileName("geomSpiRIT.root");
  fRun -> AddModule(fSpiRIT); 


  // -----------------------------------------------------------------
  // Field
  if (useFieldMapFile) {
    STFieldMap *fField = new STFieldMap("SamuraiMap_0.5T","A");
    fField -> SetPosition(-130.55/2,-51.10/2,-159.64/2);
    fRun -> SetField(fField);
  }
  else {
    FairConstField *fField = new FairConstField();
    fField -> SetField(0., 5., 0.);
    fField -> SetFieldRegion(-150, 150, -150, 150, -150, 150);
    fRun -> SetField(fField);
  }


  // -----------------------------------------------------------------
  // Event generator
  STEventGenGenerator* fEvent = new STEventGenGenerator(event);
  fEvent -> SetPrimaryVertex(0, -21.33, -3.52);

  FairPrimaryGenerator* fGenerator = new FairPrimaryGenerator();
  fGenerator -> AddGenerator(fEvent);
  fRun -> SetGenerator(fGenerator);  


  // -----------------------------------------------------------------
  // Set data base
  FairParRootFileIo* fMCPar = new FairParRootFileIo(kTRUE);
  fMCPar -> open(outParFile.Data());

  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
  fDb -> setOutput(fMCPar);
  fDb -> saveOutput();
  fDb -> print();


  // -----------------------------------------------------------------
  // Run initialization
  fRun -> SetStoreTraj(kTRUE);
  fRun -> Init();

  FairTrajFilter* trajFilter = FairTrajFilter::Instance();
  trajFilter -> SetStorePrimaries(kTRUE);
  trajFilter -> SetStoreSecondaries(kTRUE);


  // -----------------------------------------------------------------
  // Run
  fRun -> Run(fEvent -> GetNEvents());
}
