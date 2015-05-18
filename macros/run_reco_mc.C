void run_reco_mc(TString tag = "test", Int_t threshold = 1)
{
  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogFileName("log/reco_mc.log");
  logger -> SetLogToFile(kFALSE);
  logger -> SetLogToScreen(kTRUE);
  //logger -> SetLogVerbosityLevel("MEDIUM");

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile("data/spirit_" + tag + ".raw.root");
  run -> SetOutputFile("data/spirit_" + tag + ".reco.root");

  TString file = "../parameters/ST.parameters.par";

  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
  parIo1 -> open(file.Data(), "in");
  FairParRootFileIo* parIo2 = new FairParRootFileIo();
  parIo2 -> open("data/spirit_" + tag + ".params.root");
  rtdb -> setFirstInput(parIo2);
  rtdb -> setSecondInput(parIo1);

  STPSATask *psaTask = new STPSATask();
  psaTask -> SetPersistence();
  psaTask -> SetThreshold(threshold);
  psaTask -> SetPSAMode(2);
  run -> AddTask(psaTask);

  STHitClusteringTask *hcTask = new STHitClusteringTask();
  hcTask -> SetPersistence();
  //hcTask -> SetVerbose(2);
  run -> AddTask(hcTask);

  STSMTask* smTask = new STSMTask();
  smTask -> SetPersistence();
  smTask -> SetMode(STSMTask::kChange);
  run -> AddTask(smTask);

  STRiemannTrackingTask* rmTask = new STRiemannTrackingTask();
  rmTask -> SetSortingParameters(kTRUE,STRiemannSort::kSortZ,0);
  rmTask -> SetPersistence();
//  rmTask -> SetVerbose(kTRUE);
  rmTask -> SetMergeTracks(kTRUE);
  run -> AddTask(rmTask);

  run->Init();

  run->Run(0, 0);
}