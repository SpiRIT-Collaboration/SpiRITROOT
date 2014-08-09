void run_reco()
{
  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogFileName("genieLog.log");
  logger -> SetLogToFile(kTRUE);
  logger -> SetLogToScreen(kTRUE);
  logger -> SetLogVerbosityLevel("MEDIUM");

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile("data/test.mc_youngstest.root");
  run -> SetOutputFile("ha.root");

  TString file = "parameters/ST.parameters.par";

  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
  parIo1 -> open(file.Data(), "in");
  FairParRootFileIo* parIo2 = new FairParRootFileIo();
  parIo2 -> open("params_youngstest.root");
  rtdb -> setFirstInput(parIo2);
  rtdb -> setSecondInput(parIo1);

//  STDigiPar *digiPar = (STDigiPar *) rtdb -> getContainer("STDigiPar");

  STDecoderTask *decoderTask = new STDecoderTask();
  decoderTask -> SetGraw("data/cosmic_RIKEN_20140715/CoBo_AsAd0-2014-07-11T18-56-57.670_0000.graw");
  decoderTask -> SetPedestal("data/cosmic_RIKEN_20140715/CoBo_AsAd0-2014-07-11T18-56-57.670_0000.graw.root");
//  decoderTask -> SetGraw("../../../cosmic_RIKEN_20140715/CoBo_AsAd0_2014-07-11T19-34-57.035_0000.graw");
//  decoderTask -> SetPedestal("../../../cosmic_RIKEN_20140715/CoBo_AsAd0_2014-07-11T19-34-57.035_0000.graw.root");
  decoderTask -> SetNumTbs(512);
  decoderTask -> SetPersistence();
  run -> AddTask(decoderTask);

  STPSATask *psaTask = new STPSATask();
  psaTask -> SetPersistence();
  psaTask -> SetThreshold(100);
  run -> AddTask(psaTask);

  STHitClusteringTask *hcTask = new STHitClusteringTask();
  hcTask -> SetPersistence();
  hcTask -> SetVerbose(2);
  run -> AddTask(hcTask);

  run->Init();

  run->Run(0, 194);
//  run->Run(0, 194);
}
