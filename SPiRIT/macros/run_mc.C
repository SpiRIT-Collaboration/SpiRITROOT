void run_mc(const Int_t nEvents   = 1,
            TString     tag       = "urqmd1",
            TString     GeantMode = "TGeant4",
            TString     geoFile   = "spirit_v03.1.root")
{
  gRandom -> SetSeed(time(0));



  // -- Inviroment Setting -------------------------------------------------
  TString workDir   = gSystem -> Getenv("SPIRITDIR");
  TString tutdir    = workDir + "/macros";
  TString geoDir    = workDir + "/geometry";
  TString confDir   = workDir + "/gconfig";
  TString dataDir   = "data";
  TString outFile   = dataDir + "/spirit_" + tag + ".mc.root"; 
  TString parFile   = dataDir + "/spirit_" + tag + ".params.root"; 
  TString mediaFile = "media.geo";

  gSystem -> Setenv("GEOMPATH",   geoDir.Data());
  gSystem -> Setenv("CONFIG_DIR", confDir.Data());





  /** - Generator Setting --------------------------------------------------
   *
   * masses all are in MeV/c^2
   * There are two types: pions and nucleons/clusters
   * pi[0]=pi0, pi[1]=pi+ and pi-
   * nucleons are in mass[Z][A] format
   * currently only a few of the nucleons/clusters are programmed
   * will need a mass excess table for the rest
   * */

  const Int_t maxz=2;
  const Int_t maxa=4;
  Int_t ztemp;
  Int_t atemp;
  Double_t masspi[2] = {134.976, 139.570};
  Double_t mass[maxz+1][maxa+1];
  Double_t amu2mev = 931.4941;

  ifstream masses("urqmd/etable.dat");

  for(Int_t line=0; line<92; line++) {
    masses >> ztemp >> atemp;
    if(ztemp<=maxz && atemp<=maxa) 
      masses >> mass[ztemp][atemp];
  }

  for(Int_t z=0; z<=maxz; z++) {
    for(Int_t a=0; a<=maxa; a++) {
      if(mass[z][a]!=0.0) mass[z][a] = (a*1.0+mass[z][a]*1.0E-6)*amu2mev;
      else                mass[z][a] =  a*1.0*amu2mev;
    }
  }

  Int_t beamA;
  Int_t beamZ;
  Int_t targA;
  Int_t targZ;
  Int_t beamEperA;
  const Int_t events;

  ifstream urqmd("urqmd/132sn124sn300amev_urqmd_short.dat");
  urqmd >> beamA >> beamZ >> targA >> targZ >> beamEperA >> events;

  Double_t beamE  = 1.0*beamEperA*beamA+beamA*amu2mev;
  Double_t totalE = beamE+targA*amu2mev;
  Double_t beamP  = TMath::Sqrt(beamE*beamE-beamA*beamA*amu2mev*amu2mev);
  Double_t mBeta  = -beamP/totalE;

  TVector3 beta(0.,0.,-mBeta);
  TLorentzVector mLorentz;  

  //UrQMD
  vector<Int_t>    partz[events];
  vector<Int_t>    parta[events];
  vector<Double_t> pxc[events];
  vector<Double_t> pyc[events];
  vector<Double_t> pzc[events];
  vector<Double_t> pxl[events];
  vector<Double_t> pyl[events];
  vector<Double_t> pzl[events];
  vector<Int_t>    pid[events];
  vector<Double_t> kinl[events];

  Int_t    evtnum;
  Int_t    evtmult;
  Int_t    urA;
  Int_t    urZ;
  Int_t    pdg;
  Double_t px;
  Double_t py;
  Double_t pz;
  Double_t tempmass;
  Double_t totalC;
  Double_t totalL;

  for(Int_t i=1; i<=10; i++){
    urqmd >> evtnum >> evtmult;

    for(Int_t j=1; j<=evtmult; j++){
      urqmd >> urA >> urZ >> px >> py >> pz;

      if(urA<=maxa && urZ<=maxz) {
        partz[i-1].push_back(urZ);
        parta[i-1].push_back(urA);
        pxc[i-1].push_back(px);
        pyc[i-1].push_back(py);
        pzc[i-1].push_back(pz);
        if(urA==-1){
          if(urZ==0) tempmass=masspi[0];
          else       tempmass=masspi[1];
          cout << evtnum << "\t" << urZ << endl;
        }
        else tempmass=mass[urZ][urA];

        totalC = TMath::Sqrt(px*px+py*py+pz*pz+tempmass*tempmass);
        mLorentz.SetPxPyPzE(px,py,pz,totalC);
        mLorentz.Boost(beta);
        pxl[i-1].push_back(mLorentz.Px()/1000.);
        pyl[i-1].push_back(mLorentz.Py()/1000.);
        pzl[i-1].push_back(mLorentz.Pz()/1000.);
        totalL = TMath::Sqrt(  TMath::Power(mLorentz.Px(),2.)
                             + TMath::Power(mLorentz.Py(),2.)
                             + TMath::Power(mLorentz.Pz(),2.)
                             + tempmass*tempmass  );
        kinl[i-1].push_back(totalL-tempmass);

        //Setting pdg
        if (urA==-1 && urZ==1)  pdg=211;
        if (urA==-1 && urZ==0)  pdg=111;
        if (urA==-1 && urZ==-1) pdg=-211;
        if (urA==1  && urZ==0)  pdg=2112;
        if (urA==1  && urZ==1)  pdg=2212;
        if (urA==2  && urZ==1)  pdg=1000010020;
        if (urA==3  && urZ==1)  pdg=1000010030;
        if (urA==3  && urZ==2)  pdg=1000020030;
        if (urA==4  && urZ==2)  pdg=1000020040;

        pid[i-1].push_back(pdg);
      }
    }
  }




  gDebug = 0;





  // -----   Timer   --------------------------------------------------------
  TStopwatch timer;
  timer.Start();





  // -----   Logger  --------------------------------------------------------
  FairLogger *logger = FairLogger::GetLogger();
              logger -> SetLogFileName("log/MyLog.log");    // define log file name
              logger -> SetLogToScreen(kFALSE);         // log to screen and to file 
              logger -> SetLogToFile(kTRUE);
              logger -> SetLogVerbosityLevel("HIGH");   // Print very accurate output. Levels are LOW, MEDIUM and HIGH





  // -----   Create simulation run   ----------------------------------------
  FairRunSim* run = new FairRunSim();





  // -----   Create geometry   ----------------------------------------------
  FairModule* cave= new FairCave("CAVE");
              cave -> SetGeometryFileName("cave_vacuum.geo"); 

  FairDetector* spirit = new STDetector("STDetector", kTRUE);
                spirit -> SetGeometryFileName(geoFile.Data());





  // -----   Create and set magnetic field   --------------------------------
  // Constant field
  FairConstField *fMagField = new FairConstField();
                  fMagField -> SetField(0., 5., 0.); // in kG
                  fMagField -> SetFieldRegion(-(130.55+50)/2,
                                               (130.55+50)/2,
                                              -( 51.10+50)/2,
                                               ( 51.10+50)/2,
                                              -(159.64+50)/2,
                                               (159.64+50)/2);




  // -----   Create PrimaryGenerator   --------------------------------------
  Int_t dnum=7;
  const int gennum=pid[dnum].size();

  FairPrimaryGenerator *primGen = new FairPrimaryGenerator();
  FairParticleGenerator *fIongen[gennum];

  for(Int_t i=0; i<gennum; i++){
    fIongen[i] = new FairParticleGenerator(pid[dnum][i],1,
                                           pxl[dnum][i],pyl[dnum][i],pzl[dnum][i],
                                           0.0,-21.33,3.0);
    primGen -> AddGenerator(fIongen[i]);
  }





  // -----   Run initialisation   -------------------------------------------
  run -> SetName(GeantMode);              // Transport engine
  run -> SetOutputFile(outFile);          // Output file
  run -> SetWriteRunInfoFile(kFALSE);  
  run -> SetMaterials(mediaFile);      
  run -> AddModule(cave);
  run -> AddModule(spirit);
  run -> SetField(fMagField);
  run -> SetGenerator(primGen);  
  run -> SetStoreTraj(kTRUE);
  run -> Init();




 
  /**
   * Set cuts for storing the trajectories.
   * Switch this on only if trajectories are stored.
   * Choose this cuts according to your needs, but be aware
   * that the file size of the output file depends on these cuts
   *
   * Ok so if we see the line four lines above this it says 
   * Switch this on only if trajectories are stored. 
   * What that means is that there actually must be a 
   * FairTrajFilter in order to store them. 
   * It does not appear to need the SetStorePrimaries 
   * or SetStoreSecondaries to NEED to be true 
   * but I'm leaving those lines uncommented so as to make sure.
   *
   **/
  FairTrajFilter* trajFilter = FairTrajFilter::Instance();
  trajFilter -> SetStorePrimaries(kTRUE);
  trajFilter -> SetStoreSecondaries(kTRUE);





  // -----   Runtime database   ---------------------------------------------
  Bool_t kParameterMerged = kTRUE;
  FairParRootFileIo* parOut = new FairParRootFileIo(kParameterMerged);
                     parOut -> open(parFile.Data());

  FairRuntimeDb* rtdb = run -> GetRuntimeDb();
                 rtdb -> setOutput(parOut);
                 rtdb -> saveOutput();
                 rtdb -> print();





  // -----   Start run   ----------------------------------------------------
  run -> Run(nEvents);
  delete run;





  // -----   Finish   -------------------------------------------------------
  timer.Stop();
  Double_t rtime = timer.RealTime();
  Double_t ctime = timer.CpuTime();

  cout << endl << endl;
  cout << "Macro finished succesfully."  << endl;
  cout << "Output file    : " << outFile << endl;
  cout << "Parameter file : " << parFile << endl;
  cout << "Real time " << rtime << " s"  << endl;
  cout << "CPU  time " << ctime << " s"  << endl << endl;
}
