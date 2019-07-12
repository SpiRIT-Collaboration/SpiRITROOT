
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

void run_digi(TString name = "protons_75_events")
{
  gRandom -> SetSeed(0);

  // -----------------------------------------------------------------

  // FairRun
  FairRunAna* fRun = new FairRunAna();
  // -----------------------------------------------------------------
  // Set space charge task
  STSpaceChargeTask *fSpaceChargeTask = new STSpaceChargeTask();
  STFieldMap *fField = new STFieldMap("samurai_field_map","A");
  fField -> SetPosition(0., -20.43, 58.);
  fSpaceChargeTask -> SetPersistence(true);
  fSpaceChargeTask -> SetElectronDrift(false); // set this to true if you want space charge
  fSpaceChargeTask -> SetVerbose(false);
  fSpaceChargeTask -> SetBField(fField);
  fRun -> AddTask(fSpaceChargeTask);
  // Set digitization tasks
  STDriftTask* fDriftTask = new STDriftTask(); 
  fDriftTask -> SetPersistence(true);
  fDriftTask -> SetParticleForCorrection("p");
  fDriftTask -> SetSplineInterpolation(false);
  fDriftTask -> SetVerbose(false);  
  fRun -> AddTask(fDriftTask);

  STPadResponseTask* fPadResponseTask = new STPadResponseTask(); 
  fPadResponseTask -> SetPersistence(true);
  fPadResponseTask -> AssumeGausPRF();
  fRun -> AddTask(fPadResponseTask);

  STElectronicsTask* fElectronicsTask = new STElectronicsTask(); 
  fElectronicsTask -> SetPersistence(true);
  fElectronicsTask -> SetADCConstant(1.);
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


  // -----------------------------------------------------------------
  // Set file names
  TString inputFile   = dataDir + name + ".mc.root"; 
  TString outputFile  = dataDir + name + ".digi.root"; 
  TString mcParFile   = dataDir + name + ".params.root";
  TString loggerFile  = dataDir + "log_" + name + ".digi.txt";
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
  //fSpaceChargeTask -> ExportDisplacementMap("../SpaceCharge/DriftElectrons/test.root");
}
