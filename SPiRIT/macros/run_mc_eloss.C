#include "../../fairtools/FairLogger.h"
//#include "FairRunSim.h"
#include <Riostream.h>
#include <TVector3.h>
#include <TMath.h>
#include <TLorentzVector.h>

void run_mc_eloss(const Int_t nEvents = 5)
{

  gRandom -> SetSeed(time(0));
  
  TString dir = gSystem->Getenv("VMCWORKDIR");
  TString tutdir = dir + "/../SPiRIT/macros";

  TString tut_geomdir = dir + "/../SPiRIT/geometry";
  gSystem->Setenv("GEOMPATH",tut_geomdir.Data());

  TString tut_configdir = dir + "/../SPiRIT/gconfig";
  gSystem->Setenv("CONFIG_DIR",tut_configdir.Data());

  TString outDir  = "data";

  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------

  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  FairLogger *logger = FairLogger::GetLogger();
  logger->SetLogFileName("spirit.log");    // define log file name
  // log to screen and to file 
  logger->SetLogToScreen(kTRUE);
  logger->SetLogToFile(kTRUE);
  logger->SetLogVerbosityLevel("HIGH");    // Print very accurate output. Levels are LOW, MEDIUM and HIGH

  TString outFile = outDir + "/spirit_v2.root"; 
  TString parFile = outDir + "/spirit_params_v2.root"; 

    // -----   Create simulation run   ----------------------------------------
    FairRunSim* run = new FairRunSim();
    run->SetName("TGeant3");              // Transport engine
    run->SetOutputFile(outFile);          // Output file
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    // ------------------------------------------------------------------------
    
    run->SetWriteRunInfoFile(kFALSE);  
    // -----   Create media   -------------------------------------------------
    run->SetMaterials("media.geo");       // Materials
    // ------------------------------------------------------------------------
    
    // -----   Create geometry   ----------------------------------------------
    
    FairModule* cave= new FairCave("CAVE");
    cave->SetGeometryFileName("cave_vacuum.geo"); 
    run->AddModule(cave);
    
    //    FairModule* target= new FairTarget("SnTarget");
    //    target->SetGeometryFileName("target.geo"); 
    //    run->AddModule(target);
    
    FairDetector* spirit = new STDetector("STDetector", kTRUE);
    spirit->SetGeometryFileName("spirit_v03.1.root");
    run->AddModule(spirit);
    
    // -----   Create and set magnetic field   --------------------------------
    // Constant field
    FairConstField *fMagField = new FairConstField();
    fMagField -> SetField(0, 5., 0.); // in kG
    // SetFieldRegion(xmin, xmax, ymin, ymax, zmin, zmax)
   fMagField->SetFieldRegion(-(130.55+50)/2,(130.55+50)/2,-(51.10+50)/2,(51.10+50)/2,-(159.64+50)/2,(159.64+50)/2);
  

    run -> SetField(fMagField);
  
   // -----   Create PrimaryGenerator   --------------------------------------

   //      FairMCEventHeader* mcHeader = new FairMCEventHeader();
   //	  run->SetMCEventHeader(mcHeader);

     FairPrimaryGenerator* primGen = new FairPrimaryGenerator();
  //   FairUrqmdGenerator* QmdGen = new FairUrqmdGenerator("urqmd3");
  //  primGen->AddGenerator(QmdGen);
  //     run->SetGenerator(primGen);  

    Int_t particle=0;  // change the corresponding number to selects different particles

    if (particle==0) FairBoxGenerator* boxGen1 = new FairBoxGenerator(2212, 1);           //Use Proton
//  if (particle==1) FairBoxGenerator* boxGen1 = new FairBoxGenerator(1000010020, 1);     //Use Deuteron
//  if (particle==2) FairBoxGenerator* boxGen1 = new FairBoxGenerator(1000010030, 1);     //Use Triton
//  if (particle==3) FairBoxGenerator* boxGen1 = new FairBoxGenerator(1000020030, 1);     //Use Helion
//    if (particle==4) FairBoxGenerator* boxGen1 = new FairBoxGenerator(1000020040, 1);     //Use Alpha
//  if (particle==5) FairBoxGenerator* boxGen1 = new FairBoxGenerator(211, 1);            //Use Pi+
//  if (particle==6) FairBoxGenerator* boxGen1 = new FairBoxGenerator(-211, 1);           //Use Pi-
//  if (particle==7) FairBoxGenerator* boxGen1 = new FairBoxGenerator(11, 1);             //Use e-
//  if (particle==8) FairBoxGenerator* boxGen1 = new FairBoxGenerator(-11, 1);            //Use e+
//  if (particle==9) FairBoxGenerator* boxGen1 = new FairBoxGenerator(2112, 1);           //Use neutron

  boxGen1->SetPRange(0.300,0.300); //GeV/c 
  boxGen1->SetPhiRange(90.,90.); //degrees
  boxGen1->SetThetaRange(0.,0.); //degrees
  boxGen1->SetXYZ(0.,-51.01/2, 0); // cm 
  primGen->AddGenerator(boxGen1);
 
  run->SetGenerator(primGen);  
  run->SetRadLenRegister(kTRUE);
  run->SetStoreTraj(kTRUE);

  run->Init();
  // ------------------------------------------------------------------------
    
  // Set cuts for storing the trajectories.
  // Switch this on only if trajectories are stored.
  // Choose this cuts according to your needs, but be aware
  // that the file size of the output file depends on these cuts
  
  // Ok so if we see the line four lines above this it says Switch this on only if trajectories are stored. What that means is that there actually must be a FairTrajFilter in order to store them. 
  // It does not appear to need the SetStorePrimaries or SetStoreSecondaries to NEED to be true but I'm leaving those lines uncommented so as to make sure.
  FairTrajFilter* trajFilter = FairTrajFilter::Instance();
  trajFilter->SetStorePrimaries(kTRUE);
  trajFilter->SetStoreSecondaries(kTRUE);
//  trajFilter->SetStepSizeCut(0.001);  
// -----------------------------------------------------------------------
    
  // -----   Runtime database   ---------------------------------------------
  Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
  parOut->open(parFile.Data());
  rtdb->setOutput(parOut);
  rtdb->saveOutput();
  rtdb->print();
  // ------------------------------------------------------------------------
    
  // -----   Start run   ----------------------------------------------------
  run->Run(nEvents);
  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();
  cout << endl << endl;
  cout << "Macro finished succesfully." << endl;
  cout << "Output file is "    << outFile << endl;
  cout << "Parameter file is " << parFile << endl;
  cout << "Real time " << rtime << " s, CPU time " << ctime 
       << "s" << endl << endl;
  // ------------------------------------------------------------------------
}
