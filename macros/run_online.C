/**
 * Online Analysis Macro
 *
 * - This macro use only the psa method for reconstruction.
 *   If you update the event number in GUI, The analysis runs as soon as
 *   it is changed and apdate event display.
 *
 * - TODO : DOES NOT WORK WITH STDecoderTask!!!
 *
 * - How To Run
 *   In bash,
 *   > root 'run_reco.C("name", "dataFile")'
 *   You do not need to open this file to change variables.
 *
 * - Varialbles
 *   @ name : Name of simulation.
 *   @ dataFile : Full path of data file. Blanck("") for MC reconstruction.
 */

void run_online
(
  TString name     = "cosmic_short",
  TString dataFile = "../input/run_0457_event4n10.dat"
)
{
  // -----------------------------------------------------------------
  // Source
  STSource *source = new STSource();
  source -> SetData(dataFile);

  // -----------------------------------------------------------------
  // FairRun
  FairRunOnline* fRun = new FairRunOnline(source);


  // -----------------------------------------------------------------
  // Event display manager
  STEventManager *fEveManager = new STEventManager();
  fEveManager -> SetVolumeTransparency(80);


  // -----------------------------------------------------------------
  // Set reconstruction tasks
  STPSATask *fPSATask = new STPSATask();
  fPSATask -> SetThreshold(35);
  fPSATask -> SetPSAMode(STPSATask::kSimple);
  fEveManager -> AddTask(fPSATask);

  /*
  STHitClusteringTask *fClusteringTask = new STHitClusteringTask();
  fClusteringTask -> SetClusterizerMode(STHitClusteringTask::kScan2);
  //fEventManager -> AddTask(fClusteringTask);

  STSMTask* fSMTask = new STSMTask();
  fSMTask -> SetMode(STSMTask::kChange);
  //fEventManager -> AddTask(fSMTask);

  STRiemannTrackingTask* fRiemannTrackingTask = new STRiemannTrackingTask();
  fRiemannTrackingTask -> SetSortingParameters(kTRUE,STRiemannSort::kSortZ, 0);
  fEventManager -> AddTask(fRiemannTrackingTask);
  */

  STEventDrawTask* fEve = new STEventDrawTask();
  fEve -> SetRendering(STEventDrawTask::kHit, kTRUE);
  fEveManager -> AddTask(fEve);


  //////////////////////////////////////////////////////////
  //                                                      //
  //   In general, the below parts need not be touched.   //
  //                                                      //
  //////////////////////////////////////////////////////////


  // -----------------------------------------------------------------
  // Set enveiroment
  TString workDir = gSystem -> Getenv("VMCWORKDIR");
  TString dataDir = workDir + "/macros/data/";
  TString geomDir = workDir + "/geometry/";
  gSystem -> Setenv("GEOMPATH", geomDir.Data());


  // -----------------------------------------------------------------
  // Set file names
  TString inputFile   = dataDir + name + ".digi.root"; 
  TString outputFile  = dataDir + name + ".online.root"; 
  TString mcParFile   = dataDir + name + ".params.root";
  TString loggerFile  = dataDir + "log_" + name + ".reco.txt";
  TString digiParFile = workDir + "/parameters/ST.parameters.par";
  TString geoManFile  = workDir + "/geometry/geomSpiRIT.man.root";


  // -----------------------------------------------------------------
  // Logger
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger -> SetLogFileName(loggerFile);
  fLogger -> SetLogToScreen(kTRUE);
  fLogger -> SetLogToFile(kTRUE);
  fLogger -> SetLogVerbosityLevel("MEDIUM");
  fLogger -> SetLogScreenLevel("DEBUG2");


  // -----------------------------------------------------------------
  // Set FairRun
  fRun -> SetOutputFile(outputFile);


  // -----------------------------------------------------------------
  // Geometry
  //fRun -> SetGeomFile(geoManFile);
  fEveManager -> SetGeomFile(geoManFile);


  // -----------------------------------------------------------------
  // Set data base
  FairParAsciiFileIo* fDigiPar = new FairParAsciiFileIo();
  fDigiPar -> open(digiParFile);
  
  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
  fDb -> setSecondInput(fDigiPar);


  // -----------------------------------------------------------------
  // Run initialization and run
  fEveManager -> Init();
}
