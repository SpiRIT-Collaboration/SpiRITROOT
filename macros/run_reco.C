void run_reco()
{
  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogFileName("genieLog.log");
  logger -> SetLogToFile(kTRUE);
  logger -> SetLogToScreen(kTRUE);
  //logger -> SetLogVerbosityLevel("MEDIUM");

  TString workDir = gSystem->Getenv("VMCWORKDIR");
  TString geoDir  = workDir + "/geometry/";
  gSystem -> Setenv("GEOMPATH",   geoDir.Data());

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile("mc.dummy.root");
  run -> SetOutputFile("output.root");

  TString file = "../parameters/ST.parameters.par";

  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
  parIo1 -> open(file.Data(), "in");
  FairParRootFileIo* parIo2 = new FairParRootFileIo();
  parIo2 -> open("param.dummy.root");
  rtdb -> setFirstInput(parIo2);
  rtdb -> setSecondInput(parIo1);

//  STDigiPar *digiPar = (STDigiPar *) rtdb -> getContainer("STDigiPar");

  STDecoderTask *decoderTask = new STDecoderTask();
  decoderTask -> AddData("Sr90_veto_20140919_820events.graw");
  decoderTask -> SetFPNPedestal();
  decoderTask -> SetOldData(kTRUE);
//  decoderTask -> SetPedestal("../../../cosmic_RIKEN_20140715/CoBo_AsAd0-2014-07-11T18-56-57.670_0000.graw.root");
  decoderTask -> SetNumTbs(512);
  decoderTask -> SetPersistence();
  run -> AddTask(decoderTask);

  STPSATask *psaTask = new STPSATask();
  psaTask -> SetPersistence();
  psaTask -> SetThreshold(15);
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
  rmTask -> SetTrkFinderParameters(40, 10, 3, 1.0); //rmTask -> SetTrkFinderParameters(proxcut, helixcut, minpointsforfit, zStretch);
  rmTask -> SetTrkMergerParameters(40, 50, 40, 40); //rmTask -> SetTrkMergerParameters(TTproxcut, TTdipcut, TThelixcut, TTplanecut); 
  rmTask -> SetMergeTracks(kTRUE);
  run -> AddTask(rmTask);

  run->Init();

  run->Run(249, 250);
//  run->Run(0, 194);
}
