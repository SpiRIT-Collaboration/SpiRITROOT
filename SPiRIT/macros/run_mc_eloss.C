void run_mc_eloss(Int_t    nEvents  = 1,
                  Int_t    particle = 0,
                  Double_t momentum = 100, // [MeV/c]
                  TString  tag      = "eLossTest")
{
  momentum *= 0.001;
  Int_t particleID;

  if (particle==0) particleID = 2212;         // Proton
  if (particle==1) particleID = 1000010020;   // Deuteron
  if (particle==2) particleID = 1000010030;   // Triton
  if (particle==3) particleID = 1000020030;   // Helion
  if (particle==4) particleID = 1000020040;   // pha
  if (particle==5) particleID = 211;          // Pi+
  if (particle==6) particleID = -211;         // Pi-
  if (particle==7) particleID = 11;           // e-
  if (particle==8) particleID = -11;          // e+
  if (particle==9) particleID = 2112;         // neutron




  gRandom -> SetSeed(time(0));

  TString workDir = gSystem -> Getenv("SPIRITDIR");
  TString tutdir  = workDir + "/macros";
  TString geoDir  = workDir + "/geometry";
  TString confDir = workDir + "/gconfig";
  TString dataDir = "data";
  TString outFile = dataDir + "/spirit_" + tag + ".mc.root"; 
  TString parFile = dataDir + "/spirit_" + tag + ".params.root"; 

  gSystem -> Setenv("GEOMPATH",   geoDir.Data());
  gSystem -> Setenv("CONFIG_DIR", confDir.Data());

  gDebug = 0;




  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();

  FairLogger *logger = FairLogger::GetLogger();
              logger->SetLogFileName("log/spirit.log");    // define log file name
              logger->SetLogToScreen(kTRUE);
              logger->SetLogToFile(kTRUE);
              logger->SetLogVerbosityLevel("HIGH");    // Print very accurate output. Levels are LOW, MEDIUM and HIGH






  // -----   Create simulation run   ----------------------------------------
  FairRunSim* run = new FairRunSim();
  FairRuntimeDb* rtdb = run->GetRuntimeDb();





  // -----   Create geometry   ----------------------------------------------
  FairModule* cave= new FairCave("CAVE");
              cave->SetGeometryFileName("cave_vacuum.geo"); 

  FairDetector* spirit = new STDetector("STDetector", kTRUE);
                spirit->SetGeometryFileName("spirit_v03.1.root");

  FairConstField *fMagField = new FairConstField();
                  fMagField -> SetField(0, 0., 0.); // in kG
                  fMagField->SetFieldRegion(-(130.55+50)/2,
                                             (130.55+50)/2,
                                            -(51.10+50)/2,
                                             (51.10+50)/2,
                                            -(159.64+50)/2,
                                             (159.64+50)/2);

  FairBoxGenerator* boxGen1 = new FairBoxGenerator(particleID, 1);           //Use Proton
                    boxGen1->SetPRange(momentum,momentum); //GeV/c 
                    boxGen1->SetPhiRange(90.,90.); //degrees
                    boxGen1->SetThetaRange(0.,0.); //degrees
                    boxGen1->SetXYZ(0.,-51.01/2, 0); // cm 

  FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
                        primGen->AddGenerator(boxGen1);





  run -> SetName("TGeant4");              // Transport engine
  run -> SetOutputFile(outFile);          // Output file
  run -> SetWriteRunInfoFile(kFALSE);  
  run -> SetMaterials("media.geo");       // Materials
  run -> AddModule(cave);
  run -> AddModule(spirit);
  run -> SetField(fMagField);
  run -> SetGenerator(primGen);  
  run -> SetRadLenRegister(kTRUE);
  run -> SetStoreTraj(kTRUE);
  run -> Init();

  FairTrajFilter* trajFilter = FairTrajFilter::Instance();
  trajFilter->SetStorePrimaries(kTRUE);
  trajFilter->SetStoreSecondaries(kTRUE);
  //  trajFilter->SetStepSizeCut(0.001);  

  // -----   Runtime database   ---------------------------------------------
  Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
  parOut -> open(parFile.Data());
  rtdb -> setOutput(parOut);
  rtdb -> saveOutput();
  rtdb -> print();



  run -> Run(nEvents);
  delete run;



  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();

  cout << endl << endl;
  cout << "Macro finished succesfully."  << endl;
  cout << "Output file    : " << outFile << endl;
  cout << "Parameter file : " << parFile << endl;
  cout << "Real time " << rtime << " s"  << endl;
  cout << "CPU  time " << ctime << " s"  << endl << endl;
  cout << endl << endl;
}
