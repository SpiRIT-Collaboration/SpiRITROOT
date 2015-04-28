void run_eve(TString name = "test") 
{
  // Name
  TString workDir    = gSystem -> Getenv("SPIRITDIR");

  TString inputName  = "data/spirit_" + name + ".reco.root";
  TString outputName = "data/spirit_" + name + ".eve.root";
  TString parName    = "data/spirit_" + name + ".params.root";
  TString digiName   = "data/spirit_" + name + ".raw.root";
  TString digiParFile = workDir + "/parameters/ST.parameters.par";

  // Logger
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger -> SetLogToScreen(kTRUE);
  fLogger -> SetLogVerbosityLevel("HIGH");
  //fLogger -> SetLogScreenLevel("DEBUG");

  // Run
  FairRunAna *fRun= new FairRunAna();
  fRun -> SetInputFile(inputName);
  fRun -> SetOutputFile(outputName);

  // Data base 
  FairParRootFileIo* mcParInput = new FairParRootFileIo();
  mcParInput->open(parName);
  FairParAsciiFileIo* digiParInput = new FairParAsciiFileIo();
  digiParInput -> open(digiParFile);

  FairRuntimeDb* fDb = fRun->GetRuntimeDb();
  fDb -> setFirstInput(mcParInput);
  fDb -> setSecondInput(digiParInput);

  // Event display
  STEventManager *eveMan = new STEventManager();
  eveMan -> SetVolumeTransparency(80);
  STEventDrawTask* eve = new STEventDrawTask();
  eve -> SetDigiFile(digiName);
  eveMan->AddTask(eve);


  eveMan->Init();                    
}
