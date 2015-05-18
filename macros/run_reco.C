void run_reco()
{
  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogFileName("genieLog.log");
  logger -> SetLogToFile(kTRUE);
  logger -> SetLogToScreen(kTRUE);
  logger -> SetLogVerbosityLevel("MEDIUM");

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
  psaTask -> SetThreshold(40);
  psaTask -> SetPSAMode(2);
  run -> AddTask(psaTask);

  STHitClusteringTask *hcTask = new STHitClusteringTask();
  hcTask -> SetPersistence();
  hcTask -> SetVerbose(2);
  run -> AddTask(hcTask);

  STSMTask* smTask = new STSMTask();
  smTask -> SetPersistence();
  smTask -> SetMode(STSMTask::kChange);
  run -> AddTask(smTask);

  STRiemannTrackingTask* rmTask = new STRiemannTrackingTask();
  rmTask -> SetSortingParameters(kTRUE,STRiemannSort::kSortZ,0);
  rmTask -> SetPersistence();
  rmTask -> SetVerbose(kTRUE);
  rmTask -> SetMergeTracks(kTRUE);
  run -> AddTask(rmTask);

  run->Init();

  run->Run(0, 250);
//  run->Run(0, 194);
}
