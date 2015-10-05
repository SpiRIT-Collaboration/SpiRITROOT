/**
 * Reconstruction Macro
 *
 * - This macro can be used to reconstruct both MC simulation data
 *   and experiment data. For MC data reconstruction, set variable
 *   'dataFile' to blank("") - default. 
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
 *   @ dataFile : Full path of data file. Blank("") for MC reconstruction.
 *   @ dparameterFile : File name of parameter file without any path.
 *   
 */

void run_reco
(
  TString      name     = "urqmd_short",
  TString      dataFile = "",
  TString parameterFile = "ST.parameters.RIKEN_20150820.par"
)
{
  // -----------------------------------------------------------------
  // FairRun
  FairRunAna* fRun = new FairRunAna();


  // -----------------------------------------------------------------
  // Settings
  Bool_t fUseDecoder = kTRUE;
  if (dataFile.IsNull() == kTRUE)
    fUseDecoder = kFALSE;


  // -----------------------------------------------------------------
  // Set reconstruction tasks
  STDecoderTask *fDecoderTask = new STDecoderTask();
  fDecoderTask -> SetInputPersistance(kTRUE);
  fDecoderTask -> AddData(dataFile);
  fDecoderTask -> SetFPNPedestal();
  if (fUseDecoder)
    fRun -> AddTask(fDecoderTask);

  STPSATask *fPSATask = new STPSATask();
  fPSATask -> SetInputPersistance(kTRUE);
  fPSATask -> SetThreshold(30);
  fRun -> AddTask(fPSATask);

  STHitClusteringTask *fClusteringTask = new STHitClusteringTask();
  fRun -> AddTask(fClusteringTask);

  STSMTask* fSMTask = new STSMTask();
  fSMTask -> SetInputPersistance(kTRUE);
  fSMTask -> SetMode(STSMTask::kChange);
  fRun -> AddTask(fSMTask);

  STRiemannTrackingTask* fRiemannTrackingTask = new STRiemannTrackingTask();
  fRiemannTrackingTask -> SetSortingParameters(kTRUE,STRiemannSort::kSortZ, 0);
  fRiemannTrackingTask -> SetInputPersistance(kTRUE);
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
  TString digiParFile = workDir + "/parameters/" + parameterFile;
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
  if (fUseDecoder == kFALSE)
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
  if (fUseDecoder)
    fRun -> RunOnTBData();
  else
    fRun -> Run(0, 0);


  // -----------------------------------------------------------------
  // Summary
  cout << endl << endl;
  cout << "Macro finished succesfully."  << endl << endl;
  cout << "- Output file : " << outputFile << endl << endl;
}
