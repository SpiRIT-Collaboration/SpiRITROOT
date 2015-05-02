void makeDummy(const Int_t nEvents = 12000)
{
  gRandom -> SetSeed(time(0));
  
  TString dir = gSystem->Getenv("VMCWORKDIR");
  TString geomdir = dir + "/geometry";
  gSystem->Setenv("GEOMPATH", geomdir.Data());

  FairLogger *logger = FairLogger::GetLogger();
  logger->SetLogToScreen(kTRUE);
  logger->SetLogVerbosityLevel("MEDIUM");

  TString outFile = "mc.dummy.root"; 
  TString parFile = "param.dummy.root"; 

  // -----   Create simulation run   ----------------------------------------
  //
  FairRunSim* run = new FairRunSim();
  run->SetName("TGeant4");              // Transport engine
  run->SetOutputFile(outFile);          // Output file
  run->SetWriteRunInfoFile(kFALSE);  

  FairRuntimeDb* rtdb = run->GetRuntimeDb();

  // ------------------------------------------------------------------------
    
  // -----   Create geometry   ----------------------------------------------
  
  FairModule* cave= new FairCave("CAVE");
  cave->SetGeometryFileName("cave_vacuum.geo"); 
  run->AddModule(cave);
  
  FairModule* target= new FairTarget("SnTarget");
  target->SetGeometryFileName("target.geo"); 
  run->AddModule(target);
  
  FairDetector* spirit = new STDetector("STDetector", kTRUE);
  spirit->SetGeometryFileName("spirit_v03.1.root");
  run->AddModule(spirit);
    
  // ------------------------------------------------------------------------
    
  // -----   Create PrimaryGenerator   --------------------------------------
  FairPrimaryGenerator *primGen = new FairPrimaryGenerator();
  FairParticleGenerator *particleGen = new FairParticleGenerator(2112, 1, 0, 0, 0, 0, 0, 0);
  primGen->AddGenerator(particleGen);
  run->SetGenerator(primGen);  
    
  // -----   Run initialisation   -------------------------------------------
  run->Init();
  // ------------------------------------------------------------------------
    
  // -----   Runtime database   ---------------------------------------------
  Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
  parOut->open(parFile.Data());
  rtdb->setOutput(parOut);
  rtdb->saveOutput();
  // ------------------------------------------------------------------------
    
  // -----   Start run   ----------------------------------------------------
  run->Run(nEvents);
  // -----   Finish   -------------------------------------------------------
}
