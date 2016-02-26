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
 *   @ dataFile : Full path of data file.
 *                Blank("") for MC reconstruction.
 *                Ending with txt("list.txt") will use separated data files in the list file.
 *   @ parameterFile : File name of parameter file without any path.
 *   
 */

void run_reco
(
  TString          name = "run1278",
  TString      dataFile = "list_run1278.txt",
  TString parameterFile = "ST.parameters.Cosmic201602.par",
   Bool_t  useGainCalib = kFALSE
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

  Bool_t fUseSeparatedData = kFALSE;
  if (dataFile.EndsWith(".txt"))
    fUseSeparatedData = kTRUE;

  // -----------------------------------------------------------------
  // Set reconstruction tasks
  STDecoderTask *fDecoderTask = new STDecoderTask();
  fDecoderTask -> SetUseSeparatedData(fUseSeparatedData);
  fDecoderTask -> SetPersistence(kTRUE);
  fDecoderTask -> SetUseGainCalibration(useGainCalib);

  /* 
  // For manual setup of gain calibraiton
  fDecoderTask -> SetGainCalibrationData(gainCalibData);
  fDecoderTask -> SetGainReference(0.1, 0.1, 0.1);
  */

  if (!fUseSeparatedData)
    fDecoderTask -> AddData(dataFile);
  else {
    std::ifstream listFile(dataFile.Data());
    TString dataFileWithPath;
    Int_t iCobo = -1;
    while (dataFileWithPath.ReadLine(listFile)) {
      if (dataFileWithPath.Contains("s."))
        fDecoderTask -> AddData(dataFileWithPath, iCobo);
      else {
        iCobo++;
        fDecoderTask -> AddData(dataFileWithPath, iCobo);
      }
    }
  }

  if (fUseDecoder)
    fRun -> AddTask(fDecoderTask);

  STPSATask *fPSATask = new STPSATask();
  fPSATask -> SetPersistence(kTRUE);
  fPSATask -> SetThreshold(10);
  fRun -> AddTask(fPSATask);

  STCurveTrackingTask *tracking = new STCurveTrackingTask();
  tracking -> SetPersistence(kTRUE);
  fRun -> AddTask(tracking);

  STHitClusteringTask *clustering = new STHitClusteringTask();
  clustering -> SetPersistence(kTRUE);
  clustering -> SetClusterizerMode(STHitClusteringTask::kCT);
  fRun -> AddTask(clustering);

  STSMTask* fSMTask = new STSMTask();
  fSMTask -> SetPersistence(kTRUE);
  fSMTask -> SetMode(STSMTask::kChange);
  fRun -> AddTask(fSMTask);

  STRiemannTrackingTask* fRiemannTrackingTask = new STRiemannTrackingTask();
  fRiemannTrackingTask -> SetSortingParameters(kTRUE,STRiemannSort::kSortZ, 0);
  fRiemannTrackingTask -> SetPersistence(kTRUE);
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
