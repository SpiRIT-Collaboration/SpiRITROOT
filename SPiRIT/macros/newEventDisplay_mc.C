void newEventDisplay_mc() {
  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger -> SetLogToScreen(kTRUE);
  fLogger->SetLogVerbosityLevel("MEDIUM");

  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *fRun= new FairRunAna();
  fRun -> SetInputFile("data/spirit_test.reco.root");
  fRun -> SetOutputFile("data/spirit_test.reco_display.root");

  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  parIo1->open("data/spirit_test.params.root");
  rtdb->setFirstInput(parIo1);

  FairEventManager *fMan = new FairEventManager();
  STHitDraw        *hit  = new STHitDraw("STEventH", kRed, kFullDotMedium);
//  hit -> Set2DPlot();
//  hit -> Set2DPlotExternal();
//  hit -> Set2DPlotRange(47);
  fMan->AddTask(hit);

  STHitClusterDraw *cluster = new STHitClusterDraw("STEventHC", kBlue, kOpenCircle);
  fMan->AddTask(cluster);
    
  fMan->Init();                    
}
