/**
 * Event display macro
 *
 * - For detail description, see following link:
 *   https://github.com/SpiRIT-Collaboration/SpiRITROOT/wiki/Event-Display-Macro
 */

void run_eve(TString name = "urqmd_short",
             TString parameterFile = "ST.parameters.RIKEN_20150820.par",
             Bool_t online = kTRUE)
{
  // -----------------------------------------------------------------
  // FairRun
  FairRunAna* fRun = new FairRunAna();


  // -----------------------------------------------------------------
  // Set file names
  TString workDir     = gSystem -> Getenv("VMCWORKDIR");
  TString inputFile   = workDir + "/macros/data/" + name + ".reco.root";
  TString outputFile  = workDir + "/macros/data/" + name + ".eve.root";
  TString digiParFile = workDir + "/parameters/" + parameterFile;
  TString geomFile    = workDir + "/geometry/geomSpiRIT.man.root";


  // -----------------------------------------------------------------
  // Logger
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger -> SetLogToScreen(kTRUE);


  // -----------------------------------------------------------------
  // Set data base
  FairParAsciiFileIo* fDigiPar = new FairParAsciiFileIo();
  fDigiPar -> open(digiParFile);

  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
  fDb -> setSecondInput(fDigiPar);


  // -----------------------------------------------------------------
  // Set FairRun
  fRun -> SetInputFile(inputFile);
  fRun -> SetOutputFile(outputFile);
  fRun -> SetGeomFile(geomFile);


  // -----------------------------------------------------------------
  // Event display manager
  STEventManager *fEveManager = new STEventManager();
  fEveManager -> SetVolumeTransparency(80);
  fEveManager -> SetOnlineEditor(online);


  // -----------------------------------------------------------------
  // Event draw task
  STEventDrawTask* fEve = new STEventDrawTask();
  fEve -> SetRendering(STEventDrawTask::kHit, kTRUE);


  // -----------------------------------------------------------------
  // Run initialization and run
  fEveManager -> AddTask(fEve);
  fEveManager -> Init();
}
