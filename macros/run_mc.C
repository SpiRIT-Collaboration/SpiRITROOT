/**
 * Geant4 Simulation Macro
 *
 * - Geant4 settings can be found in SpiRITROOT/gconfig directory.
 *   By default, QGSP_BERT_HP_EMY physics list is used.
 *
 * - Geometry and material files can be found in SpiRITROOT/geometry
 *   directory. Make sure you run geomSpiRIT.C to create geometry
 *   file. Materials and Geant4 cuts are defined in media.geo. 
 *
 * - Default Field map is constant field. If you want to use field map
 *   file, 1) put SamuraiMap_0.5T.dat in SpiRIT/input directory and 
 *   2) set varialble 'useFieldMapFile' kTRUE. This setting may 
 *   cause longer simulation time.
 *
 * - By default, STEventGenGenerator is used for event generator.
 *   You may use other classes, but you will have to modify this code
 *   and it is not recommanded. The format of event generator file
 *   is described in STEventGenGenerator.hh.
 *
 * - How To Run
 *   In bash,
 *   > root 'run_mc.C("name", "event", kFALSE)'
 *   You do not need to open this file to change variables.
 *
 * - Varialbles
 *   @ name : Name of simulation. Just for indication.
 *   @ event : Name of event file name. 
 *   @ event : kTRUE to use constant field, with 0.5T. 
 *             kFALSE to use field map file 'SamuraiMap_0.5T.dat'.
 */

void run_mc
(
  TString name  = "urqmd_short",
  TString event = "UrQMD_300AMeV_short.egen",
  Bool_t  useFieldMapFile = kFALSE
)
{
  //////////////////////////////////////////////////////////
  //                                                      //
  //   In general, the below parts need not be touched.   //
  //                                                      //
  //////////////////////////////////////////////////////////


  // -----------------------------------------------------------------
  // Set enveiroment
  TString workDir   = gSystem -> Getenv("VMCWORKDIR");
  TString dataDir   = workDir + "/macros/data/";
  TString geomDir   = workDir + "/geometry";
  TString g4ConfDir = workDir + "/gconfig";


  // -----------------------------------------------------------------
  // Set file names
  TString outputFile = dataDir + name + ".mc.root"; 
  TString outParFile = dataDir + name + ".params.root";
  TString loggerFile = dataDir + "log_" + name + ".mc.txt";


  // -----------------------------------------------------------------
  // MC initialization
  gRandom -> SetSeed(time(0));
  gSystem -> Setenv("GEOMPATH",   geomDir.Data());
  gSystem -> Setenv("CONFIG_DIR", g4ConfDir.Data());


  // -----------------------------------------------------------------
  // Set FairRunSim
  FairRunSim* fRun = new FairRunSim();
  fRun -> SetName("TGeant4");
  fRun -> SetOutputFile(outputFile);
  fRun -> SetGenerateRunInfo(kFALSE);
  fRun -> SetMaterials("media.geo");


  // -----------------------------------------------------------------
  // Logger
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger -> SetLogFileName(loggerFile); 
  fLogger -> SetLogToScreen(kTRUE); 
  fLogger -> SetLogToFile(kTRUE);
  fLogger -> SetLogVerbosityLevel("LOW");


  // -----------------------------------------------------------------
  // Geometry
  FairModule* fCave= new FairCave("CAVE");
  fCave -> SetGeometryFileName("cave_vacuum.geo"); 
  fRun -> AddModule(fCave);

  FairDetector* fSpiRIT = new STDetector("STDetector", kTRUE);
  fSpiRIT -> SetGeometryFileName("geomSpiRIT.root");
  fRun -> AddModule(fSpiRIT); 


  // -----------------------------------------------------------------
  // Field
  if (useFieldMapFile) {
    STFieldMap *fField = new STFieldMap("SamuraiMap_0.5T","A");
    fField -> SetPosition(-130.55/2,-51.10/2,-159.64/2);
    fRun -> SetField(fField);
  }
  else {
    FairConstField *fField = new FairConstField();
    fField -> SetField(0., 5., 0.);
    fField -> SetFieldRegion(-150, 150, -150, 150, -150, 150);
    fRun -> SetField(fField);
  }


  // -----------------------------------------------------------------
  // Event generator
  STEventGenGenerator* fEvent = new STEventGenGenerator(event);
  fEvent -> SetPrimaryVertex(0, -21.33, -8.9);

  FairPrimaryGenerator* fGenerator = new FairPrimaryGenerator();
  fGenerator -> AddGenerator(fEvent);
  fRun -> SetGenerator(fGenerator);  


  // -----------------------------------------------------------------
  // Set data base
  FairParRootFileIo* fMCPar = new FairParRootFileIo(kTRUE);
  fMCPar -> open(outParFile.Data());

  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
  fDb -> setOutput(fMCPar);
  fDb -> saveOutput();
  fDb -> print();


  // -----------------------------------------------------------------
  // Run initialization
  fRun -> SetStoreTraj(kTRUE);
  fRun -> Init();

  FairTrajFilter* trajFilter = FairTrajFilter::Instance();
  trajFilter -> SetStorePrimaries(kTRUE);
  trajFilter -> SetStoreSecondaries(kTRUE);


  // -----------------------------------------------------------------
  // Run
  fRun -> Run(fEvent -> GetNEvents());


  // -----------------------------------------------------------------
  // Summary
  cout << endl << endl;
  cout << "Macro finished succesfully."  << endl << endl;
  cout << "- Output file : " << outputFile << endl << endl;
}
