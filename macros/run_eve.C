/**
 * Event display macro.
 * See following link for detail.
 * https://github.com/SpiRIT-Collaboration/SpiRITROOT/wiki/Event-Display-Macro
 */

void run_eve(TString name = "single")
{
  //Name
  TString workDir     = gSystem -> Getenv("VMCWORKDIR");
  TString inputName   = workDir + "/macros/data/spirit_" + name + ".reco.root";
  TString rawName    = workDir + "/macros/data/spirit_" + name + ".raw.root";
  TString outputName  = workDir + "/macros/data/spirit_" + name + ".eve.root";
  TString digiParName = workDir + "/parameters/ST.parameters.par";
  TString geomName    = workDir + "/geometry/geomSpiRIT.man.root";

  //Logger
  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(kTRUE);
  logger -> SetLogVerbosityLevel("HIGH");
  logger -> SetLogScreenLevel("DEBUG");

  // Run
  FairRunAna *run= new FairRunAna();
  FairRuntimeDb* db = run -> GetRuntimeDb();
  FairParAsciiFileIo* par = new FairParAsciiFileIo();
  par -> open(digiParName);
  db -> setSecondInput(par);

  run -> SetInputFile(inputName);
  run -> AddFriend(rawName);
  run -> SetOutputFile(outputName);
  run -> SetGeomFile(geomName);

  // Event display
  STEventManager *eveMan = new STEventManager();
  eveMan -> SetVolumeTransparency(80);

  // Draw
  STEventDrawTask* eve = new STEventDrawTask();
  eve -> SetRendering(STEventDrawTask::kHit,        kFALSE, 0, 4095);
  eve -> SetRendering(STEventDrawTask::kCluster,    kFALSE, 0, 4095*20);
  eve -> SetRendering(STEventDrawTask::kClusterBox, kFALSE, 0, 4095*20);
  eve -> SetRendering(STEventDrawTask::kRiemann,    kTRUE,  0, 4095*20);      
  //eve -> SetAttributes(STEventDrawTask::kHit,     kFullCircle, 0.5,  kRed-7);
  //eve -> SetAttributes(STEventDrawTask::kCluster, kFullCircle, 1, kBlack);
  //eve -> SetAttributes(STEventDrawTask::kRiemann, kFullCircle, 1);

  eveMan -> AddTask(eve);
  eveMan -> Init();
}
