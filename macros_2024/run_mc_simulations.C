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
//#include "TDatabasePDG.h"
//#include "/imports/tool/STDatabasePDG.hh"
//#include "/home/jesus/Desktop/SpROOT/sim_2025/imports/tool/STDatabasePDG.hh"
//#include "TDatabasePDG.h"
//#include "/mnt/home/lokotkot/SpiRITROOT/format/STMCPoint.hh"

/*************************************************************
* Functions needed to create directory if it is non-existance*
* Only call recursive_mkdir. There's no need to call rek_mkdir   *
**************************************************************/
void rek_mkdir(char* path);
void recursive_mkdir(const char* t_path);
//************End of Directory creation function****************
void add_pdg();
//zch 6/6/2025 void AddIons(FairRunSim *fRun, TString event);

void run_mc_simulations
(
 TString name  = "local_test",
 Int_t   nEvent = -1,
 Double_t tPartMomentum = 0,
 Short_t tPartType = 0,
 Short_t tMF_Settings = 0,
 TString inputDir = "",
 TString outputDir = "./data/Simulations/",
 Bool_t  useFieldMapFile = kTRUE,
 TString parName = "Generator.config.par",
 Int_t   nSplit=0
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
  fRun -> SetName("TGeant4");
  fRun -> SetOutputFile(outputFile);
  fRun -> SetGenerateRunInfo(kFALSE);
  fRun -> SetMaterials("media.geo");
  //AddIons(fRun, event);


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

  //------------------------------------------------------------------
  // Aux Header
  STAuxHeaderTask *auxHead = new STAuxHeaderTask();
  auxHead -> SetAuxBranch("MCAuxHeader"); //Space Charge task in digi is expecting this to be named MCAuxHeader
  auxHead -> SetPersistence();
  fRun -> AddTask(auxHead);

  // -----------------------------------------------------------------
  // Field
  if (useFieldMapFile && tMF_Settings > 0) {
    STFieldMap *fField = new STFieldMap("samurai_field_map","A");
    switch (tMF_Settings) {
    case 1: fField -> SetPosition(0., -20.43, 58.); break;
    case 2: fField -> SetPosition(-0.1794, -20.5502, 58.0526); break;
    case 3: fField -> SetPosition(0., -20.43, 58.); break;
   
    default:  cout << "Field position not set up for case " << tMF_Settings;  
    }
    fRun -> SetField(fField);
  }
  else {
    cout<<"Loading default (constant) field settings B=5"<<endl;
    FairConstField *fField = new FairConstField();
    fField -> SetField(0., 5., 0.);
    fField -> SetFieldRegion(-150, 150, -150, 150, -150, 150);
    fRun -> SetField(fField);
  }


  // -----------------------------------------------------------------
  // Event generator
  //STSimpleEventGenerator* fEvent = new STSimpleEventGenerator("../input/GEN_singleTrack.sgen");
  //STSimpleEventGenerator* fEvent = new STSimpleEventGenerator();
  //fEvent->ReadConfig((parDir + parName).Data());
  //fEvent -> SetPrimaryVertex(0, -21.33, -.89);
  //fEvent -> SetCocktailEvent(300.);
  //fEvent -> SetAngleStep(2212, numevent, 0.5, 0., 85., 180., 180.); // (pid, #evt, p, theta_begin, theta_end, phi_begin, phi_end[deg])

  
  // TString inputFile = name + ".root";
  // STTransportModelEventGenerator* fEvent;
  // if(inputDir.IsNull())
  //   fEvent = new STTransportModelEventGenerator(inputDir, inputFile);
  // else
  //   fEvent = new STTransportModelEventGenerator(inputFile);
  // fEvent->RegisterHeavyIon();
  // fEvent->SetPrimaryVertex(TVector3(0.,-21.33,-.89));


  add_pdg();
  auto fEvent = new STSingleTrackGenerator();
  fEvent->RegisterHeavyIon();
  //fEvent->ReadConfig((parDir + parName).Data()); //Use this if you want to read in information from a config file (Vertex File MUST be stores in SpiRITROOT/parameters/.)
  //fEvent->SetVertexFile("./parameters/VertexLocation.txt"); //(Vertex file can be stored anywhere, but you must manually set the number of events)
  //fEvent->SetAuxHeaderTask(auxHead); //Turn on if using ReadConfig or SetVertexFile to ensure the correct TPC event number is being stored in the Aux header (needed for Space Charge in digi and reco)
  fEvent->SetVertexBegin(nSplit);
  //fEvent->SetPrimaryVertex(0, -21.33, -.89); //cm <<This is the original values from the base script
  fEvent->SetPrimaryVertex(0, -19.00, -2.1); //cm <<This is Curtis's updated values, close to the center of the target from Spring 2024
  //fEvent->SetCocktailEvent(200.);
  //fEvent->SetParticleList({211, 2212,1000010020,1000010030,1000020030,1000020040,1000030060,1000030070,1000040070});
  int tPID=0;
  cout << "tPartType = "<<tPartType<<endl;
  switch (tPartType) {
  case 1: 
    tPID = 2212;
    cout << "running simulations for protons"<<endl;
    break;
  case 2: 
    tPID = 1000010020;
    cout << "running simulations for deuterons"<<endl;
    break;
  case 3: 
    tPID = 1000010030;
    cout << "running simulations for tritons"<<endl;
    break;
  case 4: 
    tPID = 1000020040;
    cout << "running simulations for alphas"<<endl;
    break;
  case 5: 
    tPID = 211;
    cout << "running simulations for pion_plus"<<endl;
    break;
  case 6: 
    tPID = -211;
    cout << "running simulations for pion_minus"<<endl;
    break;
  case 7: 
    tPID = 1000030060;
    cout << "running simulations for 6Li"<<endl;
    break;
  case 8: 
    tPID = 1000030070;
    cout << "running simulations for 7Li"<<endl;
    break;
  default:
    cout << "particle type not defined"<<endl;
    break;
  }
    fEvent->SetParticleList({tPID});
  //fEvent->SetRandomMomentum(true, 0.0, 2.0);
  TVector3 m(0.0, 0.0, tPartMomentum);
  fEvent->SetMomentum(m);
  //fEvent->SetUniformRandomDirection(true);


  FairPrimaryGenerator* fGenerator = new FairPrimaryGenerator();
  fGenerator -> AddGenerator(fEvent);
  fRun -> SetGenerator(fGenerator);
  //AddIons(fRun, fEvent);

  // -----------------------------------------------------------------
  // Set data base
  
  FairParRootFileIo* fMCPar = new FairParRootFileIo(kTRUE); //
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
  fRun -> Run( nEvent==-1 ? fEvent->GetNEvents() : nEvent );

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

void add_pdg()
{
  TDatabasePDG* pdgDB = TDatabasePDG::Instance();

  cout<<"Adding more partiles"<<endl;
  const Double_t kAu2Gev = 0.9314943228;
  const Double_t khSlash = 1.0545726663e-27;
  const Double_t kErg2Gev = 1/1.6021773349e-3;
  const Double_t khShGev = khSlash*kErg2Gev;
  const Double_t kYear2Sec = 3600*24*365.25;

  pdgDB->AddParticle("Deuteron","Deuteron",2*kAu2Gev+8.071e-3,kTRUE, 0,3,"Ion",1000010020);
  pdgDB->AddParticle("Triton","Triton",3*kAu2Gev+14.931e-3,kFALSE, khShGev/(12.33*kYear2Sec),3,"Ion",1000010030);
  pdgDB->AddParticle("Alpha","Alpha",4*kAu2Gev+2.424e-3,kTRUE, khShGev/(12.33*kYear2Sec),6,"Ion",1000020040);
  pdgDB->AddParticle("HE3","HE3",3*kAu2Gev+14.931e-3,kFALSE, 0,6,"Ion",1000020030);
}
