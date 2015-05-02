void makeDummy(const Int_t nEvents = 20000)
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

  run->SetMaterials("media.geo");      
    
  // -----   Create PrimaryGenerator   --------------------------------------
  FairPrimaryGenerator *primGen = new FairPrimaryGenerator();
  FairParticleGenerator *particleGen = new FairParticleGenerator(2112, 1, 0, 0, 0, 0, 0, 0);
  primGen->AddGenerator(particleGen);
  run->SetGenerator(primGen);  
  /*
  Int_t dnum=7;
  const int gennum=pid[dnum].size();
  cout << gennum << endl;
  FairParticleGenerator *fIongen[gennum];
  
  for (Int_t i=0; i<gennum; i++) {
    fIongen[i] = new FairParticleGenerator(pid[dnum][i],1,pxl[dnum][i],pyl[dnum][i],pzl[dnum][i],0.0,-21.33,-3.52);
    //cout << pid[dnum][i] << "\t" << pxl[dnum][i] << "\t" << pyl[dnum][i] << "\t" << pzl[dnum][i] << "\t" << kinl[dnum][i] << endl;
    primGen->AddGenerator(fIongen[i]);
  }


    //primGen->AddTrack(pdg,pionpC[0][i],pionpC[1][i],pionpC[2][i],pionrC[0][i]*1.0E-13,pionrC[1][i]*1.0E-13-21.33,pionrC[2][i]*1.0E-13-3.52);
    //fIongen[i] = new FairParticleGenerator(pdg,1,pionpC[0][i],pionpC[1][i],pionpC[2][i],pionrC[0][i]*1.0E-13,pionrC[1][i]*1.0E-13-21.33,pionrC[2][i]*1.0E-13-3.52);
  //fIongen[i] = new FairParticleGenerator(pdg,1,px,py,pz,0.0,-21.33,-3.52);
  //primGen->AddGenerator(fIongen[i]);
    //primGen->AddTrack(pdg,pionpC[0][i],pionpC[1][i],pionpC[2][i],pionrC[0][i]*1.0E-13,pionrC[1][i]*1.0E-13-21.33,pionrC[2][i]*1.0E-13-3.52);
    //}
  //}

  // if (partt==0) FairBoxGenerator* boxGen1 = new FairBoxGenerator(2212, 1);
  // if (partt==1) FairBoxGenerator* boxGen1 = new FairBoxGenerator(1000010020, 1);
  // if (partt==2) FairBoxGenerator* boxGen1 = new FairBoxGenerator(1000010030, 1);
  // if (partt==3) FairBoxGenerator* boxGen1 = new FairBoxGenerator(1000020030, 1);
  // if (partt==4) FairBoxGenerator* boxGen1 = new FairBoxGenerator(1000020040, 1);
  // if (partt==5) FairBoxGenerator* boxGen1 = new FairBoxGenerator(211, 1);
  // if (partt==6) FairBoxGenerator* boxGen1 = new FairBoxGenerator(-211, 1);
  // if (partt==7) FairBoxGenerator* boxGen1 = new FairBoxGenerator(11, 1);
  // if (partt==8) FairBoxGenerator* boxGen1 = new FairBoxGenerator(-11, 1);
  // //FairBoxGenerator* boxGen1 = new FairBoxGenerator(1000020040, 1);
  // boxGen1->SetPRange(0.25,0.5); //GeV/c ??
  // boxGen1->SetPhiRange(0.,360.); //degrees
  // boxGen1->SetThetaRange(0.,90.); //degrees
  // boxGen1->SetXYZ(0.,-21.33, -3.52); // cm 
  // primGen->AddGenerator(boxGen1);
    
  // ------------------------------------------------------------------------
    
  run->SetStoreTraj(kTRUE);
  */
    
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
