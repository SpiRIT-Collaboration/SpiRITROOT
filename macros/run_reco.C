/**
 * Reconstruction macro
 */

void run_reco
(
  TString name     = "urqmd_short",
  TString dataFile = "",         // blank("") for MC simulation
  Int_t   nEvents  = 0           // 0 for MC simulation
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
  fDecorderTask -> SetWindow(512, 0);
  fDecorderTask -> SetNumTbs(512);
  if (fUseDecorderTask)
    fRun -> AddTask(fDecorderTask);

  STPSATask *fPSATask = new STPSATask();
  fPSATask -> SetPersistence();
  fPSATask -> SetThreshold(15);
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
  fRiemannTrackingTask -> SetMergeTracks(kTRUE);
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
    fRun -> SetInputFile(inputFile.Data());
  fRun -> SetOutputFile(outputFile.Data());


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
  fRun -> Run(0, 0);
}
