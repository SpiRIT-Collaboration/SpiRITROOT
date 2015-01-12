void run_digi(TString tag = "test")
{
  TStopwatch timer;
  timer.Start();

  Int_t nEvents = 1;

  TString workDir     = gSystem -> Getenv("SPIRITDIR");
  TString geomDir     = workDir + "/geometry";
  TString confDir     = workDir + "/gconfig";
  TString dataDir     = "data";
  TString mcFile      = dataDir + "/spirit_" + tag + ".mc.root"; 
  TString mcParFile   = dataDir + "/spirit_" + tag + ".params.root"; 
  TString digiFile    = dataDir + "/spirit_" + tag + ".raw.root"; 
  TString digiParFile = workDir + "/parameters/ST.parameters.par";

  gSystem->Setenv("GEOMPATH",geomDir.Data());
  gSystem->Setenv("CONFIG_DIR",confDir.Data());



  FairLogger *logger = FairLogger::GetLogger();
              logger -> SetLogFileName("log/digi.log");
              logger -> SetLogToScreen(kTRUE);
              logger -> SetLogToFile(kTRUE);
              logger -> SetLogVerbosityLevel("HIGH");

  FairRunAna* fRun = new FairRunAna();
              fRun -> SetInputFile(mcFile.Data());
              fRun -> SetOutputFile(digiFile.Data());

  FairParRootFileIo*  mcParInput = new FairParRootFileIo();
                      mcParInput -> open(mcParFile);
  FairParAsciiFileIo* digiParInput = new FairParAsciiFileIo();
                      digiParInput -> open(digiParFile);
  
  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
                 fDb -> setFirstInput(mcParInput);
                 fDb -> setSecondInput(digiParInput);

  STDriftTask* drift = new STDriftTask();
  STPadResponseTask* padResponse = new STPadResponseTask();
  STElectronicsTask* electronics = new STElectronicsTask();
                     electronics -> SetSignalPolarity(1);




  fRun -> AddTask(drift);
  fRun -> AddTask(padResponse);
  fRun -> AddTask(electronics);
  fRun -> Init();
  fRun -> Run(0, nEvents);

  timer.Stop();
  cout << endl << endl;
  cout << "Digi macro finished succesfully." << endl;
  cout << "Output file : " << digiFile       << endl;
  cout << "Real time " << timer.RealTime()   << " s" << endl;
  cout << "CPU  time " << timer.CpuTime()    << " s" << endl << endl;
}
