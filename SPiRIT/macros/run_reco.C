void run_reco(TString filename)
{
  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogFileName("run_reco.log");
  logger -> SetLogToFile(kTRUE);
  logger -> SetLogToScreen(kTRUE);
  logger -> SetLogVerbosityLevel("MEDIUM");

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile("mc.dummy.root");
  TString outputFile = filename;
  outputFile.ReplaceAll(".graw", ".reco.root");
  run -> SetOutputFile(outputFile.Data());

  TString file = "../parameters/ST.parameters.par";

  FairRuntimeDb* rtdb = run->GetRuntimeDb();
  FairParAsciiFileIo* parIo1 = new FairParAsciiFileIo();
  parIo1 -> open(file.Data(), "in");
  FairParRootFileIo* parIo2 = new FairParRootFileIo();
  parIo2 -> open("param.dummy.root");
  rtdb -> setFirstInput(parIo2);
  rtdb -> setSecondInput(parIo1);

  STDigiPar *digiPar = (STDigiPar *) rtdb -> getContainer("STDigiPar");

  STDecoderTask *decoderTask = new STDecoderTask();
  decoderTask -> AddData(filename.Data());
  decoderTask -> SetFPNPedestal();
  decoderTask -> SetNumTbs(digiPar -> GetNumTbs());
  decoderTask -> SetPersistence();
  run -> AddTask(decoderTask);

  STPSATask *psaTask = new STPSATask();
  psaTask -> SetPersistence();
  psaTask -> SetPSAMode(2);
  psaTask -> SetThreshold(50);
  run -> AddTask(psaTask);

  run->Init();

  run->Run(0, 20000);
//  run->Run(0, 194);
}
