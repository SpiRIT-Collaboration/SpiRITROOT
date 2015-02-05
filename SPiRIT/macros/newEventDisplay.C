void newEventDisplay(TString filename) {
  if (!filename.Contains(".reco.root")) {
    cerr << " = Please use file having '.reco.root' processed by run_reco.C!" << endl;

    return;
  }

  FairLogger *fLogger = FairLogger::GetLogger();
  fLogger -> SetLogToScreen(kTRUE);
  fLogger->SetLogVerbosityLevel("MEDIUM");

  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *fRun= new FairRunAna();
  fRun -> SetInputFile(filename.Data());
  filename.ReplaceAll(".reco.root", ".display.root");
  fRun -> SetOutputFile(filename.Data());

  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  parIo1->open("param.dummy.root");
  rtdb->setFirstInput(parIo1);

  FairEventManager   *fMan      = new FairEventManager();
  STHitDraw          *hit       = new STHitDraw("STEventH", kRed, kFullDotMedium);
  hit -> Set2DPlot();
  hit -> Set2DPlotExternal();
//  hit -> Set2DPlotRange(47);
  fMan->AddTask(hit);
    
  fMan->Init();                    
}
