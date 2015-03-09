void run_eve(TString tag = "test") 
{
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger -> SetLogToScreen(kTRUE);
  fLogger->SetLogVerbosityLevel("MEDIUM");

  FairRunAna *fRun= new FairRunAna();
  fRun -> SetInputFile("data/spirit_" + tag + ".reco.root");
  fRun -> SetOutputFile("data/spirit_" + tag + ".reco_display.root");

  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  parIo1->open("data/spirit_" + tag + ".params.root");
  rtdb->setFirstInput(parIo1);

  FairRootManager* ioman = FairRootManager::Instance();

  STEventManager *eveMan = new STEventManager();
  STEventDrawTask* eve = new STEventDrawTask();

  eveMan->AddTask(eve);
  eveMan->Init();                    
}
