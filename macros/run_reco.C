/**
 * Reconstruction Macro
 *
 * - This macro can be used to reconstruct both MC simulation data
 *   and experiment data. For MC data reconstruction, set variable
 *   'dataFile' to blanck("") - default. 
 *
 * - See headers of each tasks for more information.
 *
 * - How To Run
 *   In bash,
 *   > root 'run_reco.C("name", "dataFile", 0)'
 *   You do not need to open this file to change variables.
 *
 * - Varialbles
 *   @ name : Name of simulation.
 *   @ dataFile : Full path of data file. Blanck("") for MC reconstruction.
 *   
 */

void run_reco
(
  TString name     = "urqmd_short",
  TString dataFile = ""
)
{
  // -----------------------------------------------------------------
  // FairRun
  FairRunAna* fRun = new FairRunAna();


  // -----------------------------------------------------------------
  // Settings
  Bool_t fUseDecorderTask = kTRUE;
  if (dataFile.IsNull() == kTRUE)
    fUseDecorderTask = kFALSE;


  // -----------------------------------------------------------------
  // Set reconstruction tasks
  STDecoderTask *fDecorderTask = new STDecoderTask();
  fDecorderTask -> SetPersistence();
  fDecorderTask -> AddData(dataFile);
  fDecorderTask -> SetFPNPedestal(100);
  fDecorderTask -> SetWindow(256, 300);
  fDecorderTask -> SetNumTbs(512);
  if (fUseDecorderTask)
    fRun -> AddTask(fDecorderTask);

  STPSATask *fPSATask = new STPSATask();
  fPSATask -> SetPersistence();
  fPSATask -> SetThreshold(25);
  fRun -> AddTask(fPSATask);

  STHitClusteringTask *fClusteringTask = new STHitClusteringTask();
  fClusteringTask -> SetPersistence();
  fRun -> AddTask(fClusteringTask);

  STSMTask* fSMTask = new STSMTask();
  fSMTask -> SetPersistence();
  fSMTask -> SetMode(STSMTask::kChange);
  fRun -> AddTask(fSMTask);

  STRiemannTrackingTask* fRiemannTrackingTask = new STRiemannTrackingTask();
  fRiemannTrackingTask -> SetSortingParameters(kTRUE,STRiemannSort::kSortZ, 0);
  fRiemannTrackingTask -> SetPersistence();
  fRun -> AddTask(fRiemannTrackingTask);


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
  TString outputFile  = dataDir + name + ".reco.root"; 
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
  fLogger -> SetLogVerbosityLevel("LOW");


  // -----------------------------------------------------------------
  // Set FairRun
  if (fUseDecorderTask == kFALSE)
    fRun -> SetInputFile(inputFile);
  fRun -> SetOutputFile(outputFile);


  // -----------------------------------------------------------------
  // Geometry
  fRun -> SetGeomFile(geoManFile);


  // -----------------------------------------------------------------
  // Set data base
  FairParAsciiFileIo* fDigiPar = new FairParAsciiFileIo();
  fDigiPar -> open(digiParFile);
  
  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
  fDb -> setSecondInput(fDigiPar);


  // -----------------------------------------------------------------
  // Run initialization
  fRun -> Init();


  // -----------------------------------------------------------------
  // Run
  if (fUseDecorderTask)
    fRun -> RunOnTBData();
  else
    fRun -> Run(0, 0);


  // -----------------------------------------------------------------
  // Summary
  cout << endl << endl;
  cout << "Macro finished succesfully."  << endl << endl;
  cout << "- Output file : " << outputFile << endl << endl;
}
