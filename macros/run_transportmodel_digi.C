/**
 * Digitization Simulation Macro
 *
 * - Modifying this macro is not recommanded.
 *
 * - How To Run
 *   In bash,
 *   > root 'run_digi.C("name")'
 *   You do not need to open this file to change variables.
 *
 * - Varialbles
 *   @ name : Name of simulation. Should be same with MC simulation.
 */

void run_transportmodel_digi
(
 TString name = "urqmd_short",
 TString inputDir = "",
 TString outputDir = ""
)
{
  // -----------------------------------------------------------------
  // FairRun
  FairRunAna* fRun = new FairRunAna();

  gRandom -> SetSeed(0);

  // -----------------------------------------------------------------
  // Set digitization tasks
	
  STAnalyzeG4StepTask* fAnaG4StepTask = new STAnalyzeG4StepTask();
  fAnaG4StepTask -> SetPersistence(false);
  fAnaG4StepTask -> SetTAPersistence(true);
  fAnaG4StepTask -> AssumeGausPRF();
  fAnaG4StepTask -> SetGainMatchingData(false);
  fRun -> AddTask(fAnaG4StepTask);
  	
  STElectronicsTask* fElectronicsTask = new STElectronicsTask(); 
  fElectronicsTask -> SetPersistence(true);
  fElectronicsTask -> SetADCConstant(1);
  fElectronicsTask -> SetPulseData("pulser_117ns_50tb.dat");
  fElectronicsTask -> SetUseSaturationTemplate(true);
//  fElectronicsTask -> SetTbRange(30,257);
  fRun -> AddTask(fElectronicsTask);


  //////////////////////////////////////////////////////////
  //                                                      //
  //   In general, the below parts need not be touched.   //
  //                                                      //
  //////////////////////////////////////////////////////////


  // -----------------------------------------------------------------
  // Set enveiroment
  TString workDir   = gSystem -> Getenv("VMCWORKDIR");
  TString dataDir   = workDir + "/macros/data/";

  if(inputDir.IsNull())
    inputDir = dataDir;
  if(outputDir.IsNull())
    outputDir = dataDir;

  // -----------------------------------------------------------------
  // Set file names
  TString inputFile   = inputDir  + name + ".mc.root"; 
  TString mcParFile   = inputDir  + name + ".params.root";
  TString outputFile  = outputDir + name + ".digi.root"; 
  TString loggerFile  = outputDir + "log_" + name + ".digi.txt";
  TString digiParFile = workDir + "/parameters/ST.parameters.par";


  // -----------------------------------------------------------------
  // Set FairRun
  fRun -> SetInputFile(inputFile.Data());
  fRun -> SetOutputFile(outputFile.Data());


  // -----------------------------------------------------------------
  // Logger
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger -> SetLogFileName(loggerFile);
  fLogger -> SetLogToScreen(kTRUE);
  fLogger -> SetLogToFile(kTRUE);
  fLogger -> SetLogVerbosityLevel("LOW");


  // -----------------------------------------------------------------
  // Set data base
  FairParRootFileIo* fMCPar = new FairParRootFileIo();
  fMCPar -> open(mcParFile);
  FairParAsciiFileIo* fDigiPar = new FairParAsciiFileIo();
  fDigiPar -> open(digiParFile);
  
  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
  fDb -> setFirstInput(fMCPar);
  fDb -> setSecondInput(fDigiPar);


  // -----------------------------------------------------------------
  // Run initialization
  fRun -> Init();


  // -----------------------------------------------------------------
  // Run
  fRun -> Run(0,0);


  // -----------------------------------------------------------------
  // Summary
  cout << endl << endl;
  cout << "Macro finished succesfully."  << endl << endl;
  cout << "- Output file : " << outputFile << endl << endl;
}
