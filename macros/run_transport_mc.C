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
#include <sys/stat.h>
#include <sys/types.h>

/*************************************************************
* Functions needed to create directory if it is non-existance*
* Only call recursive_mkdir. There's no need to call rek_mkdir   *
**************************************************************/
void rek_mkdir(char* path);
void recursive_mkdir(const char* t_path);
//************End of Directory creation function****************

void run_transport_mc
(
 TString name  = "Run_2841_mc_low_energy",
 Int_t   nEvent = -1,
 TString inputDir = "",
 TString outputDir = "data/",
 Bool_t  useFieldMapFile = kTRUE,
 Int_t   start_evt=0,
 std::set<int> allowedParticles={}
)
{
  //////////////////////////////////////////////////////////
  //                                                      //
  //   In general, the below parts need not be touched.   //
  //                                                      //
  //////////////////////////////////////////////////////////
  
  
  // -----------------------------------------------------------------
  // Set environment
  TString workDir   = gSystem -> Getenv("VMCWORKDIR");
  TString geomDir   = workDir + "/geometry";
  TString g4ConfDir = workDir + "/gconfig";
  TString dataDir   = workDir + "/macros/data/";
  TString parDir    = workDir + "/parameters/";

  if(outputDir.IsNull())
    outputDir = dataDir;

  // -----------------------------------------------------------------
  // Set file names
  TString outputFile = outputDir + name + ".mc.root"; 
  TString outParFile = outputDir + name + ".params.root";
  TString loggerFile = outputDir + "log_" + name + ".mc.txt";

  // ----------------------------------------------------------------
  // Create directories for all the output if they are non-existent
  recursive_mkdir(outputFile.Data());
  recursive_mkdir(outParFile.Data());
  recursive_mkdir(loggerFile.Data());


  // -----------------------------------------------------------------
  // MC initialization
  gRandom -> SetSeed(0);
  gSystem -> Setenv("GEOMPATH",   geomDir.Data());
  gSystem -> Setenv("CONFIG_DIR", g4ConfDir.Data());


  // -----------------------------------------------------------------
  // Set FairRunSim
  FairRunSim* fRun = new FairRunSim();
  fRun -> SetMCEventHeader(new STFairMCEventHeader());
  fRun -> SetName("TGeant4");
  fRun -> SetOutputFile(outputFile);
  fRun -> SetGenerateRunInfo(kTRUE);
  fRun -> SetMaterials("media.geo");
//  AddIons(fRun, event);

  // -----------------------------------------------------------------
  // Set data base
  FairParRootFileIo* fMCPar = new FairParRootFileIo(kTRUE);
  fMCPar -> open(outParFile.Data());

  FairRuntimeDb* fDb = fRun -> GetRuntimeDb();
  fDb -> setOutput(fMCPar);
  fDb -> saveOutput();
  fDb -> print();




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
    STFieldMap *fField = new STFieldMap("samurai_field_map","A");
    fField -> SetPosition(0., -20.43, 58.);
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
  auto fEvent = new STModelToLabFrameGenerator("imqmd_CM.root");
  fEvent -> RegisterHeavyIon(allowedParticles);
  fEvent -> SetBeamAndTarget(0.270, 132, 50, 124); 
  fEvent -> SetPrimaryVertex(TVector3(0.04,-20.55,-1.32));
  fEvent -> SetVertexXYSigma(TVector2(0.406, 0.362));
  fEvent -> SetTargetThickness(0.08);
  fEvent -> SetBeamDetectorVertexSigma(TVector2(0.0708, 0.0265));
  fEvent -> SetBeamAngle(TVector2(-0.0443, 0.00086));
  fEvent -> SetBeamAngleSigma(TVector2(0.00224, 0.00382));
  fEvent -> SetBeamDetectorAngleSigma(TVector2(0.00064, 0.00024));

  fEvent -> SetMaxAllowedZ(2);
  fEvent -> SetMaxMult(50);
  fEvent -> SetStartEvent(start_evt);

  FairPrimaryGenerator* fGenerator = new FairPrimaryGenerator();
  fGenerator -> AddGenerator(fEvent);
  fRun -> SetGenerator(fGenerator);  


  
  // -----------------------------------------------------------------
  // Run initialization
  fRun -> SetStoreTraj(kTRUE);
  fRun -> Init();

  FairTrajFilter* trajFilter = FairTrajFilter::Instance();
  trajFilter -> SetStorePrimaries(kTRUE);
  trajFilter -> SetStoreSecondaries(kTRUE);


  // -----------------------------------------------------------------
  // Run
  fRun -> Run( nEvent==-1 ? fEvent->GetNEvents() : nEvent );


  // -----------------------------------------------------------------
  // Summary
  cout << endl << endl;
  cout << "Macro finished succesfully."  << endl << endl;
  cout << "- Output file : " << outputFile << endl << endl;
}

void rek_mkdir(char* path)
{
  char *sep = strrchr(path, '/' );
  if(sep != NULL) {
    *sep = 0;
    rek_mkdir(path);
    *sep = '/';
  }
  if( mkdir(path,0755) && errno != EEXIST )
    printf("error while trying to create '%s'\n%m\n",path ); 
}

void recursive_mkdir(const char* t_path)
{
    char* path = new char[strlen(t_path) + 1];
    strcpy(path, t_path);
    char *sep = strrchr(path, '/' );
    if(sep ) { 
       char *path0 = strdup(path);
       path0[ sep - path ] = 0;
       rek_mkdir(path0);
       free(path0);
    }

    delete[] path;
}
