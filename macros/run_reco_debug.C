void run_reco_debug(TString tag = "single")
{
  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogFileName("log/reco_mc.log");
  logger -> SetLogToFile(kFALSE);
  logger -> SetLogToScreen(kTRUE);
  logger -> SetLogVerbosityLevel("LOW");

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile("data/spirit_" + tag + ".raw.root");
  run -> SetOutputFile("data/spirit_" + tag + ".reco.root");

  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
  parIo1 -> open("../parameters/ST.parameters.par", "in");
  FairParRootFileIo* parIo2 = new FairParRootFileIo();
  parIo2 -> open("data/spirit_" + tag + ".params.root");
  rtdb -> setFirstInput(parIo2);
  rtdb -> setSecondInput(parIo1);

  STPSATask *psaTask = new STPSATask();
  psaTask -> SetPersistence();
  psaTask -> SetThreshold(5);
  psaTask -> SetPSAMode(1);
  run -> AddTask(psaTask);

  STHitClusteringTask *hcTask = new STHitClusteringTask();
  hcTask -> SetPersistence();
  hcTask -> SetClusterizerMode(2);
  hcTask -> SetVerbose(2);
  //hcTask -> SetProximityCut(...);
  //hcTask -> SetSigmaCut(...);
  run -> AddTask(hcTask);

  STSMTask* smTask = new STSMTask();
  smTask -> SetPersistence();
  smTask -> SetMode(STSMTask::kChange);
  run -> AddTask(smTask);

  STRiemannTrackingTask* rmTask = new STRiemannTrackingTask();
  rmTask -> SetSortingParameters(kTRUE,STRiemannSort::kSortR,0);
  rmTask -> SetPersistence();
  //rmTask -> SetVerbose(kTRUE);
  //rmTask -> SetMergeTracks(kTRUE);
  rmTask -> SetTrkFinderParameters(10000, 10000, 3, 1.0); //rmTask -> SetTrkFinderParameters(proxcut, helixcut, minpointsforfit, zStretch);
  //rmTask -> SetTrkMergerParameters(40, 50, 40, 40); //rmTask -> SetTrkMergerParameters(TTproxcut, TTdipcut, TThelixcut, TTplanecut); 
  rmTask -> SetMaxRMS(100);
  run -> AddTask(rmTask);

  //STGenfitTask* genTask = new STGenfitTask();
  //run -> AddTask(genTask);

  run->Init();
  run->Run(0, 0);

  STDebugLogger::Instance() -> Write();
}
