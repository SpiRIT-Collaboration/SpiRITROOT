void run_mc(const Int_t nEvents = 200)
{

  gRandom -> SetSeed(time(0));
  
  TString tut_geomdir = "geometry";
  gSystem->Setenv("GEOMPATH",tut_geomdir.Data());

  //masses all are in MeV/c^2
  //There are two types: pions and nucleons/clusters
  //pi[0]=pi0, pi[1]=pi+ and pi-
  //nucleons are in mass[Z][A] format
  //currently only a few of the nucleons/clusters are programmed
  //will need a mass excess table for the rest
  const Int_t maxz=2, maxa=4;
  Double_t masspi[2];
  Double_t mass[maxz+1][maxa+1];
  masspi[0]=134.976;
  masspi[1]=139.570;
  Double_t amu2mev=931.4941;
  ifstream masses("etable.dat");
  Int_t ztemp, atemp;
  for (Int_t line=0; line<92; line++) {
    masses >> ztemp >> atemp;
    if (ztemp<=maxz && atemp<=maxa) masses >> mass[ztemp][atemp];
  }
  for (Int_t z=0; z<=maxz; z++) {
    for (Int_t a=0; a<=maxa; a++) {
      if (mass[z][a]!=0.0) mass[z][a]=(a*1.0+mass[z][a]*1.0E-6)*amu2mev;
      else mass[z][a]=a*1.0*amu2mev;
    }
  }

  ifstream urqmd("132sn124sn300amev_urqmd_short.dat");
  Int_t beamA, beamZ, targA, targZ, beamEperA;
  const Int_t events;
  urqmd >> beamA >> beamZ >> targA >> targZ >> beamEperA >> events;
  Double_t beamE=1.0*beamEperA*beamA+beamA*amu2mev;
  Double_t totalE=beamE+targA*amu2mev;
  Double_t beamP=TMath::Sqrt(beamE*beamE-beamA*beamA*amu2mev*amu2mev);
  Double_t mBeta=-beamP/totalE;
  TVector3 beta(0.,0.,-mBeta);
  TLorentzVector mLorentz;  

  //UrQMD
  vector<int> partz[events];
  vector<int> parta[events];
  vector<double> pxc[events];
  vector<double> pyc[events];
  vector<double> pzc[events];
  vector<double> pxl[events];
  vector<double> pyl[events];
  vector<double> pzl[events];
  vector<int> pid[events];
  vector<double> kinl[events];

  Int_t evtnum, evtmult;;
  Double_t px, py, pz;
  Int_t urA, urZ;
  Double_t tempmass;
  Double_t totalC, totalL;
  Int_t pdg;

  for (Int_t i=1; i<=10; i++) {
    urqmd >> evtnum >> evtmult;
    //cout << evtnum << "\t" << evtmult << endl;
    for (Int_t j=1; j<=evtmult; j++) {
      urqmd >> urA >> urZ >> px >> py >> pz;
      if (urA<=maxa && urZ<=maxz) {
	partz[i-1].push_back(urZ);
	parta[i-1].push_back(urA);
	pxc[i-1].push_back(px);
	pyc[i-1].push_back(py);
	pzc[i-1].push_back(pz);
	if (urA==-1) {
	  if (urZ==0) tempmass=masspi[0];
	  else tempmass=masspi[1];
	  cout << evtnum << "\t" << urZ << endl;
	}
	else {
	  tempmass=mass[urZ][urA];
	}
	totalC=TMath::Sqrt(px*px+py*py+pz*pz+tempmass*tempmass);
	mLorentz.SetPxPyPzE(px,py,pz,totalC);
	mLorentz.Boost(beta);
	pxl[i-1].push_back(mLorentz.Px()/1000.);
	pyl[i-1].push_back(mLorentz.Py()/1000.);
	pzl[i-1].push_back(mLorentz.Pz()/1000.);
	totalL=TMath::Sqrt(TMath::Power(mLorentz.Px(),2.)+TMath::Power(mLorentz.Py(),2.)+TMath::Power(mLorentz.Pz(),2.)+tempmass*tempmass);
	kinl[i-1].push_back(totalL-tempmass);
	//Setting pdg
	if (urA==-1 && urZ==1) pdg=211;
	if (urA==-1 && urZ==0) pdg=111;
	if (urA==-1 && urZ==-1) pdg=-211;
	if (urA==1 && urZ==0) pdg=2112;
	if (urA==1 && urZ==1) pdg=2212;
	if (urA==2 && urZ==1) pdg=1000010020;
	if (urA==3 && urZ==1) pdg=1000010030;
	if (urA==3 && urZ==2) pdg=1000020030;
	if (urA==4 && urZ==2) pdg=1000020040;
	pid[i-1].push_back(pdg);
      }
    }
  }

  // In general, the following parts need not be touched
  // ========================================================================

  // ----    Debug option   -------------------------------------------------
  gDebug = 0;
  // ------------------------------------------------------------------------

  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();
  // ------------------------------------------------------------------------

  FairLogger *logger = FairLogger::GetLogger();
  logger->SetLogFileName("MyLog.log");    // define log file name
  // log to screen and to file 
  logger->SetLogToScreen(kTRUE);
  logger->SetLogToFile(kTRUE);
  logger->SetLogVerbosityLevel("HIGH");    // Print very accurate output. Levels are LOW, MEDIUM and HIGH

  TString outFile = "test.mc_youngstest.root"; 
  TString parFile = "params_youngstest.root"; 

    // -----   Create simulation run   ----------------------------------------
    FairRunSim* run = new FairRunSim();
    run->SetName("TGeant4");              // Transport engine
    run->SetOutputFile(outFile);          // Output file
    FairRuntimeDb* rtdb = run->GetRuntimeDb();
    // ------------------------------------------------------------------------
    
    run->SetWriteRunInfoFile(kFALSE);  
    // -----   Create media   -------------------------------------------------
    //  run->SetMaterials("media.geo");       // Materials
    //run->SetMaterials("media_pnd.geo");       // Materials
    // ------------------------------------------------------------------------
    
    // -----   Create geometry   ----------------------------------------------
    
    FairModule* cave= new FairCave("CAVE");
    cave->SetGeometryFileName("cave_vacuum.geo"); 
    run->AddModule(cave);
    
    FairModule* target= new FairTarget("SnTarget");
    target->SetGeometryFileName("target.geo"); 
    run->AddModule(target);
    
    FairDetector* spirit = new STDetector("STDetector", kTRUE);
    //spirit->SetGeometryFileName("spirit.geo"); 
    //spirit->SetGeometryFileName("tpc_prototype_ArCo2.root"); 
    spirit->SetGeometryFileName("testingsave_geom.root");
    run->AddModule(spirit);
    
    // ------------------------------------------------------------------------
    
    // -----   Create and set magnetic field   --------------------------------
    // Constant field
    FairConstField *fMagField = new FairConstField();
    fMagField -> SetField(0., 5., 0.); // in kG
    // SetFieldRegion(xmin, xmax, ymin, ymax, zmin, zmax)
    fMagField -> SetFieldRegion(-147.74/2, 147.74/2, -53.93/2, 53.93/2, 0, 200.98);
    run -> SetField(fMagField);
    // ------------------------------------------------------------------------
    
    // -----   Create PrimaryGenerator   --------------------------------------
  FairPrimaryGenerator *primGen = new FairPrimaryGenerator();
  run->SetGenerator(primGen);  
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
    
  // -----   Run initialisation   -------------------------------------------
  //if (i==0) run->Init();
  run->Init();
  // ------------------------------------------------------------------------
    
  // Set cuts for storing the trajectories.
  // Switch this on only if trajectories are stored.
  // Choose this cuts according to your needs, but be aware
  // that the file size of the output file depends on these cuts
    
  //FairTrajFilter* trajFilter = FairTrajFilter::Instance();
  //trajFilter->SetStepSizeCut(0.01); // 1 cm
  //trajFilter->SetStorePrimaries(kTRUE);
  //trajFilter->SetStoreSecondaries(kTRUE);
  // ------------------------------------------------------------------------
    
  // -----   Runtime database   ---------------------------------------------
  //if (i==0) {
  Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
  parOut->open(parFile.Data());
  rtdb->setOutput(parOut);
  //}
  rtdb->saveOutput();
  rtdb->print();
  
  // ------------------------------------------------------------------------
    
  // -----   Start run   ----------------------------------------------------
  run->Run(nEvents);
  //}

  // ------------------------------------------------------------------------
  //  run->CreateGeometryFile("data/geofile_full.root");
  
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

  cout << mBeta << endl;

}
