/**
 * Online Analysis Macro
 *
 * - This macro use only the psa method for reconstruction.
 *   If you update the event number in GUI, The analysis runs as soon as
 *   it is changed and apdate event display.
 *
 * - How To Run
 *   In bash,
 *   > root 'run_reco.C("name", "dataFile")'
 *   You do not need to open this file to change variables.
 *
 * - Varialbles
 *   @ name : Name of simulation.
 *   @ dataFile : Full path of data file.
 *   @ parameterFile : name of the digi par.
 */

void run_online
(
  TString          name = "cosmic_short",
  TString      dataFile = "",
  TString parameterFile = "ST.parameters.RIKEN_20151021.par",
   Bool_t  useGainCalib = kFALSE
)
{
  // -----------------------------------------------------------------
  // Source
  STSource *source = new STSource();
  source -> SetData(dataFile);
  if (useGainCalib)
    source -> SetUseGainCalibration();

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
  TString digiParFile = workDir + "/parameters/" + parameterFile;
  TString geoManFile  = workDir + "/geometry/geomSpiRIT.man.root";


  // -----------------------------------------------------------------
  // Logger
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger -> SetLogFileName(loggerFile);
  fLogger -> SetLogToScreen(kTRUE);
  fLogger -> SetLogToFile(kTRUE);
  fLogger -> SetLogVerbosityLevel("MEDIUM");


  // -----------------------------------------------------------------
  // Set FairRun
  fRun -> SetOutputFile(outputFile);
  fRun -> SetAutoFinish(kFALSE);


  // -----------------------------------------------------------------
  // Geometry
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
