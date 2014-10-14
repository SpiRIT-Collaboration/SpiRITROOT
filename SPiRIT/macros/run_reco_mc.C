void run_reco_mc(TString tag = "test")
{
  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogFileName("log/reco_mc.log");
  logger -> SetLogToFile(kTRUE);
  logger -> SetLogToScreen(kTRUE);
  logger -> SetLogVerbosityLevel("MEDIUM");

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile("data/spirit_" + tag + ".raw.root");
  run -> SetOutputFile("data/spirit_" + tag + ".reco.root");

  TString file = "../parameters/ST.parameters.par";

  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
  parIo1 -> open(file.Data(), "in");
  FairParRootFileIo* parIo2 = new FairParRootFileIo();
  parIo2 -> open("param.dummy.root");
  rtdb -> setFirstInput(parIo2);
  rtdb -> setSecondInput(parIo1);

  STPSATask *psaTask = new STPSATask();
  psaTask -> SetPersistence();
  psaTask -> SetThreshold(10);
  run -> AddTask(psaTask);

  STHitClusteringTask *hcTask = new STHitClusteringTask();
  hcTask -> SetPersistence();
  hcTask -> SetVerbose(2);
  run -> AddTask(hcTask);

  run->Init();

  run->Run(0, 1);
}
