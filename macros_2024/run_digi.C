
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

// zch 2/19/2025 simulateBeam was set to true by default
void run_digi(TString name = "test",
              Int_t fRunNo = 0,
              bool simulateBeam = false,
              Short_t tMF_Settings = 0,
              double beamrate = -1,
              // zch
              TString ParFile = "ST.parameters.par",
              TString GainFile = "RelativeGain_Same.list")
// TString ParFile = "ST.parameters.fullmc.par")
{
  gRandom->SetSeed(0);

  TString workDir = gSystem->Getenv("VMCWORKDIR");

  //++++++++++++++++++++++++++++++++++++//
  //+++++||FOR SPACE CHARGE TASK||++++++//
  //++++++++++++++++++++++++++++++++++++//

  //=====Set SAMURAI B-field offsets=====//
  auto fFieldOffsetX = -0.1794;
  auto fFieldOffsetY = -20.5502;
  auto fFieldOffsetZ = 58.0526;
  //=====End SAMURAI B-field offsets=====//

  //=====Set Space Charge Constants=====//
  double fSCslope = 0;
  double fSCinter = 0;

  if(fRunNo < 1600) {
    fSCslope = -1.1307e-11;
    fSCinter = 1.9819e-9;
  }
  else {
    fSCslope = -2.2799e-11;
    fSCinter = 2.9456e-9;
  }
  fSCslope = 0;
  fSCinter = 0;
  //=====End Space Charge Constants=====//

  //=====Set Rate File Location=====//
  auto fRateFile = TString::Format("RatesSup/run%04d.root", fRunNo);
  //=====End Rate File Location=====//

  //+++++||END SPACE CHARGE||+++++//

  //++++++++++++++++++++++++++//
  //+++++||FOR Drift TASK||+++++//
  //++++++++++++++++++++++++++//

  //=====Obtain run dependent Drift Velocity=====//

  double fDriftVelocity = 5.45;
  ifstream dvFile;
  if(fRunNo < 1600)
    dvFile.open("driftVelocity/driftVelocitySummary124Xe.txt");
  else
    dvFile.open("driftVelocity/driftVelocitySummary136Xe.txt");
  if(!dvFile.is_open()) {
    cout << "Error: Drift Velocity file not found!" << endl;
    cout << "Aborting" << endl;
    gApplication->Terminate();
  }
  int runN;
  double DV;
  int counts;
  bool dvGood = false;
  while(dvFile >> runN >> DV >> counts) {
    if(runN == fRunNo) {
      fDriftVelocity = DV;
      dvGood = true;
      continue;
    }
  }
  if(!dvGood) {
    cout << "Error: Run " << fRunNo << " is not in the drift velocity file!" << endl;
    cout << "Aborting" << endl;
    gApplication->Terminate();
  }
  //=====Finished Drift Velocity=====//

  //+++++||END Drift||+++++//

  //++++++++++++++++++++++++++//
  //+++++||FOR Pad Response TASK||+++++//
  //++++++++++++++++++++++++++//

  //=====Obtain run dependent Time Offset (TB_0 != Ypos_0)=====//

  double fGGtb = 33;
  ifstream ggTBFile;
  if(fRunNo < 1600)
    ggTBFile.open("driftVelocity/GGD_tb_124Xe.txt");
  else
    ggTBFile.open("driftVelocity/GGD_tb_136Xe.txt");

  if(!ggTBFile.is_open()) {
    cout << "Error: GG TB file not found!" << endl;
    cout << "Aborting" << endl;
    gApplication->Terminate();
  }
  double TB;
  bool tbGood = false;
  while(ggTBFile >> runN >> TB) {
    if(runN == fRunNo) {
      fGGtb = TB;
      tbGood = true;
      continue;
    }
  }
  if(!tbGood) {
    cout << "Error: Run " << fRunNo << " is not in the GG TB file!" << endl;
    cout << "Aborting" << endl;
    gApplication->Terminate();
  }
  double fTBTime = 40;
  double fTbToYConv = -fTBTime * fDriftVelocity / 100;
  double fGGpos = fGGtb * fTbToYConv;
  double fYPedestalOffset = fGGpos - 16 + 4;
  double fTimeOffset = fYPedestalOffset / (fDriftVelocity / 100);
  cout << "Y Offset: " << fYPedestalOffset << endl;
  cout << "Time Offset: " << fTimeOffset << endl;
  //=====Finished Time Offset=====//

  //=====Set Gain Calibration=====//
  TString gainCalibFile = workDir + "/parameters/gainCalibration_groundPlane_2024spring_threshold_10.root";
  double constant = 8.065E-2;
  double linear = 2.352E-3;
  double quadratic = -4.789E-8;
  //=====End Gain Calibration=====//

  //+++++||END Pad Response||+++++//

  // -----------------------------------------------------------------

  // FairRun
  FairRunAna *fRun = new FairRunAna();
  // -----------------------------------------------------------------

  // Aux Header
  STAuxHeaderTask *auxHead = new STAuxHeaderTask();
  auxHead -> SetInputBranch("MCAuxHeader");
  auxHead -> SetAuxBranch("DigiAuxHeader");
  auxHead -> SetPersistence();
  fRun -> AddTask(auxHead);

  // Set space charge task
  // uncomment the following to enable space charge
  STSpaceChargeTask *fSpaceChargeTask = new STSpaceChargeTask();
  FairField *fField = nullptr; // Declare fField as a pointer to FairField
  switch (tMF_Settings)
  {
    case 0: 
    fField = new FairConstField();
    dynamic_cast<FairConstField*>(fField)->SetField(0., 5., 0.);
    // Remove the SetPosition call for FairConstField
    dynamic_cast<FairConstField*>(fField)->SetFieldRegion(-150, 150, -150, 150, -150, 150);
    break;
    case 1:
    fField = new STFieldMap("samurai_field_map", "A");
    dynamic_cast<STFieldMap*>(fField)->SetPosition(0., -20.43, 58.);
    break;
  case 2:
    fField = new STFieldMap("samurai_field_map", "A");
    dynamic_cast<STFieldMap*>(fField)->SetPosition(-0.1794, -20.5502, 58.0526);
    break;
    case 3:
    fField = new STFieldMap("samurai_field_map", "A");
    dynamic_cast<STFieldMap*>(fField)->SetPosition(0., -20.43, 58.);
    break;

    default:
    cout << "Field position not set up for case " << tMF_Settings;
  }

  fSpaceChargeTask->SetBField(fField);
  fSpaceChargeTask->SetPersistence(false);
  fSpaceChargeTask->SetVerbose(false);
  if(fRunNo < 1600)
    fSpaceChargeTask -> SetProjectile(STSpaceCharge::Projectile::Xe124); // Spring
  else
    fSpaceChargeTask -> SetProjectile(STSpaceCharge::Projectile::Xe136); // Fall
  //fSpaceChargeTask->SetProjectile(STSpaceCharge::Projectile::Sn132); // changed from Sn132 to Xe124 to test 07/16/2025 // No projectile dependence on rigidity effect
  //fSpaceChargeTask->SetSheetChargeDensity(beamrate); // sheet charge density for run 2899
  fSpaceChargeTask -> SetLocalRate(fSCslope, fSCinter, fRateFile);

  // Set digitization tasks
  STDriftTask *fDriftTask = new STDriftTask(); // all tasks are set to false, I need to check this 07/16/2025
  fDriftTask->SetPersistence(false);
  fDriftTask->SetSplineInterpolation(false);
  fDriftTask->SetVerbose(false);
  fDriftTask->SetDriftVelocity(fDriftVelocity);
  fDriftTask->SetYDriftOffset(fTimeOffset);

  STPadResponseTask *fPadResponseTask = new STPadResponseTask();
  fPadResponseTask->SetPersistence(false);
  fPadResponseTask->AssumeGausPRF();
  fPadResponseTask -> SetElectronicsJitterFile(workDir + "/parameters/yOffsetCalibration.dat");
  fPadResponseTask->SetGainMatchingData(gainCalibFile.Data());
  fPadResponseTask->SetDriftVelocity(fDriftVelocity / 100.);
  //fPadResponseTask->SetTimeOffset(fTimeOffset);

  /*******************************************************************************
  // This class simulates dead pads due to drift electrons from the beam
  // You don't need this if you want to do embedding
  // But if you are running a full Transport model simulation and you want the correct efficiency you need this
  ********************************************************************************/

  STSimulateBeamTask *beamTask = new STSimulateBeamTask();
  beamTask->SetDeadPadOnBeam(workDir + "/input/ProbDeadPad.root", "Sn132");
  beamTask->SetHeavyFragments(workDir + "/SpaceCharge/potential/_132Sn_BeamTrack.data", -203.3, 5000000, 4.3); // changed from Sn132 to Xe124 to test 07/16/2025 // No projectile dependence on rigidity effect

  STElectronicsTask *fElectronicsTask = new STElectronicsTask();
  fElectronicsTask->SetPersistence(true);
  fElectronicsTask->SetADCConstant(1.);
  fElectronicsTask->SetGainMatchingData(gainCalibFile.Data());

  //////////////////////////////////////////////////////////
  //                                                      //
  //   In general, the below parts need not be touched.   //
  //                                                      //
  //////////////////////////////////////////////////////////

  // -----------------------------------------------------------------
  // Set enveiroment
  // TString dataDir   = workDir + "/macros/data/";
  TString dataDir = "./data_sim/";

  // -----------------------------------------------------------------
  // Set file names
  TString inputFile = dataDir + name + ".mc.root";
  TString outputFile = dataDir + name + ".digi.root";
  TString mcParFile = dataDir + name + ".params.root";
  TString loggerFile = dataDir + "log_" + name + ".digi.txt";
  TString digiParFile = workDir + "/parameters/" + ParFile;

  // -----------------------------------------------------------------
  // Set FairRun
  fRun->SetInputFile(inputFile.Data());
  fRun->SetOutputFile(outputFile.Data());

  // -----------------------------------------------------------------
  // Logger
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger->SetLogFileName(loggerFile);
  fLogger->SetLogToScreen(kTRUE);
  fLogger->SetLogToFile(kTRUE);
  fLogger->SetLogVerbosityLevel("LOW");

  // -----------------------------------------------------------------
  // Set data base
  FairParRootFileIo *fMCPar = new FairParRootFileIo();
  fMCPar->open(mcParFile);
  FairParAsciiFileIo *fDigiPar = new FairParAsciiFileIo();
  fDigiPar->open(digiParFile);

  FairRuntimeDb *fDb = fRun->GetRuntimeDb();
  fDb->setFirstInput(fMCPar);
  fDb->setSecondInput(fDigiPar);

  //=====||Add Tasks to Run||=====//
  fRun->AddTask(fSpaceChargeTask);
  fRun->AddTask(fDriftTask);
  fRun->AddTask(fPadResponseTask);
  if (simulateBeam) 
    fRun->AddTask(beamTask);
  fRun->AddTask(fElectronicsTask);
  //=====||End Add Tasks||=====//

  // -----------------------------------------------------------------
  // Run initialization
  fRun->Init();

  // -----------------------------------------------------------------
  // Run
  fRun->Run(0, 0);
  //fRun->Run(0, 1);

  // -----------------------------------------------------------------
  // Summary
  cout << endl
       << endl;
  cout << "Macro finished succesfully." << endl
       << endl;
  cout << "- Output file : " << outputFile << endl
       << endl;
}
