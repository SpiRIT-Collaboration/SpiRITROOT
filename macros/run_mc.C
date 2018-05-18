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

void AddIons(FairRunSim *fRun, TString event);

void run_mc
(
  TString name  = "urqmd_short",
  //TString event = "urqmd_132sn124sn270amevb0012_10event.dat",
  TString event = "urqmd_132sn124sn270amevb0012_10event.root",
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
//  AddIons(fRun, event);


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
  //STEventGenGenerator* fEvent = new STEventGenGenerator(event);
  STTransportModelEventGenerator* fEvent = new STTransportModelEventGenerator(event);
  fEvent -> RegisterHeavyIon();
  fEvent -> SetPrimaryVertex(TVector3(0, -21.33, -.89));

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
  fRun -> Run(3);


  // -----------------------------------------------------------------
  // Summary
  cout << endl << endl;
  cout << "Macro finished succesfully."  << endl << endl;
  cout << "- Output file : " << outputFile << endl << endl;
}

void AddIons(FairRunSim *fRun, TString event)
{
  TString symbol[50] = {"H", "He", "Li", "Be", "B", "C", "N", "O", "F", "Ne",
                        "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar", "K", "Ca",
                        "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn",
                        "Ga", "Ge", "As", "Se", "Br", "Kr", "Rb", "Sr", "Y", "Zr",
                        "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn"};

  TString input_dir = gSystem -> Getenv("VMCWORKDIR");
  TString fGenFileName = input_dir + "/input/" + event;
  LOG(INFO) << "Opening EventGen file for adding ions: " << fGenFileName <<FairLogger::endl;

  ifstream fGenFile(fGenFileName.Data());
  if(!fGenFile.is_open())
    LOG(FATAL) << "Cannont open EventGen file: " << fGenFileName << FairLogger::endl;

  vector<Int_t> ions;
  Int_t nEvents, pdg, eventID, nTracks;
  Double_t tmp, b;

  fGenFile >> nEvents;

  for (Int_t iEvent = 0; iEvent < nEvents; iEvent++) {
    fGenFile >> eventID >> nTracks >> b;

    for(Int_t iTrack = 0; iTrack < nTracks; iTrack++) {
      fGenFile >> pdg >> tmp >> tmp >> tmp;
      if (pdg > 3000)
        ions.push_back(pdg);
    }
  }

  fGenFile.close();

  std::sort(ions.begin(), ions.end());
  ions.resize(std::distance(ions.begin(), std::unique(ions.begin(), ions.end())));

  for (Int_t iIon = 0; iIon < ions.size(); iIon++) {
    auto z = (ions.at(iIon)%10000000)/10000;
    auto a = (ions.at(iIon)%10000)/10;

    fRun -> AddNewIon(new FairIon(Form("%d", a) + symbol[z - 1], z, a, z));
  }
}
