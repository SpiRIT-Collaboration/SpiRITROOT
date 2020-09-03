
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

void run_digi(TString name = "protons_75_events", 
	      TString dataDir = "",
	      double beamrate = 3.909E-08, // run 2900
	      TString ParFile = "ST.parameters.par", 
	      bool simulateBeam = false)
{
  gRandom -> SetSeed(0);


  // -----------------------------------------------------------------

  // FairRun
  FairRunAna* fRun = new FairRunAna();
  // -----------------------------------------------------------------
  // Set space charge task
  // uncomment the following to enable space charge
  STSpaceChargeTask *fSpaceChargeTask = new STSpaceChargeTask();
  auto fField = new STFieldMap("samurai_field_map","A");
  fField -> SetPosition(0., -20.43, 58.);
  fSpaceChargeTask -> SetBField(fField);
  fSpaceChargeTask -> SetPersistence(false);
  fSpaceChargeTask -> SetVerbose(false);
  fSpaceChargeTask -> SetProjectile(STSpaceCharge::Projectile::Sn132);
  fSpaceChargeTask -> SetSheetChargeDensity(beamrate); // sheet charge density for run 2899
  if(beamrate >= 0) fRun -> AddTask(fSpaceChargeTask);


  // Set digitization tasks
  STDriftTask* fDriftTask = new STDriftTask(); 
  fDriftTask -> SetPersistence(false);
  fDriftTask -> SetSplineInterpolation(false);
  fDriftTask -> SetVerbose(false);  
  fRun -> AddTask(fDriftTask);

  TString workDir   = gSystem -> Getenv("VMCWORKDIR");
  STPadResponseTask* fPadResponseTask = new STPadResponseTask(); 
  fPadResponseTask -> SetPersistence(false);
  fPadResponseTask -> AssumeGausPRF();
  fPadResponseTask -> SetElectronicsJitterFile(workDir + "/parameters/yOffsetCalibration.dat");
  fPadResponseTask -> SetGainMatchingData(workDir + "/parameters/RelativeGain.list");
  fRun -> AddTask(fPadResponseTask);

  /*******************************************************************************
  // This class simulates dead pads due to drift electrons from the beam
  // You don't need this if you want to do embedding
  // But if you are running a full Transport model simulation and you want the correct efficiency you need this
  ********************************************************************************/

  STSimulateBeamTask* beamTask = new STSimulateBeamTask();
  beamTask -> SetDeadPadOnBeam(workDir + "/input/ProbDeadPad.root", "Sn132");
  beamTask -> SetHeavyFragments(workDir + "/SpaceCharge/potential/_132Sn_BeamTrack.data", -203.3, 5000000, 4.3);
  if(simulateBeam) fRun -> AddTask(beamTask);


  STElectronicsTask* fElectronicsTask = new STElectronicsTask(); 
  fElectronicsTask -> SetPersistence(true);
  fElectronicsTask -> SetADCConstant(1.);
  fElectronicsTask -> SetGainMatchingData(workDir + "/parameters/RelativeGain.list");
  fRun -> AddTask(fElectronicsTask);


  //////////////////////////////////////////////////////////
  //                                                      //
  //   In general, the below parts need not be touched.   //
  //                                                      //
  //////////////////////////////////////////////////////////


  // -----------------------------------------------------------------
  // Set enveiroment
  //  TString dataDir   = workDir + "/macros/data/";


  // -----------------------------------------------------------------
  // Set file names
  TString inputFile   = dataDir + name + ".mc.root"; 
  TString outputFile  = dataDir + name + ".digi.root"; 
  TString mcParFile   = dataDir + name + ".params.root";
  TString loggerFile  = dataDir + "log_" + name + ".digi.txt";
  TString digiParFile = workDir + "/parameters/" + ParFile;


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
