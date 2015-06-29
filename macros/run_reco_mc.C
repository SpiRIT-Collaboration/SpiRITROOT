void run_reco_mc(TString tag = "test", Int_t threshold = 1)
{
  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogFileName("log/reco_mc.log");
  logger -> SetLogToFile(kFALSE);
  logger -> SetLogToScreen(kTRUE);
  logger -> SetLogVerbosityLevel("LOW");

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
  psaTask -> SetPSAMode(1);
  run -> AddTask(psaTask);

  STHitClusteringTask *hcTask = new STHitClusteringTask();
  hcTask -> SetPersistence();
  hcTask -> SetClusterizerMode(1);
  hcTask -> SetVerbose(2);
  /** 
   * i-th par: explanation
   * 0: z-reach cut in z-padsize unit
   * 1: x-reach cut in x-padsize unit
   * 2: y-reach cut in timebucket unit
   * 3: x-sigma cut in x-padsize unit
   * 4: y-sigma cut in timebucket unit
   */
  Double_t par[5] = {1.5, 4.5, 2.5, 1.8, 1};
  hcTask -> SetParameters(par);
  run -> AddTask(hcTask);

  STSMTask* smTask = new STSMTask();
  smTask -> SetPersistence();
  smTask -> SetMode(STSMTask::kChange);
  run -> AddTask(smTask);

  STRiemannTrackingTask* rmTask = new STRiemannTrackingTask();
  rmTask -> SetSortingParameters(kTRUE,STRiemannSort::kSortZ,0);
  rmTask -> SetPersistence();
  //rmTask -> SetVerbose(kTRUE);
  rmTask -> SetMergeTracks(kTRUE);
  rmTask -> SetTrkFinderParameters(40, 10, 3, 1.0); //rmTask -> SetTrkFinderParameters(proxcut, helixcut, minpointsforfit, zStretch);
  rmTask -> SetTrkMergerParameters(40, 50, 40, 40); //rmTask -> SetTrkMergerParameters(TTproxcut, TTdipcut, TThelixcut, TTplanecut); 
  run -> AddTask(rmTask);

  run->Init();

  run->Run(0, 0);
  //run->Run(0, 0);
}
