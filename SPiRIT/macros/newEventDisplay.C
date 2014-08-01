void newEventDisplay() {
  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *fRun= new FairRunAna();
  fRun -> SetInputFile("ha.root");
  fRun -> SetOutputFile("data/test.root");

  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();
  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  parIo1->open("data/params_youngstest.root");
  rtdb->setFirstInput(parIo1);

  FairEventManager   *fMan      = new FairEventManager();
  STHitDraw          *hit       = new STHitDraw("STEventH", kRed, kFullSquare);
  hit -> Set2DPlot();
  hit -> Set2DPlotExternal();
  hit -> Set2DPlotRange(47);
                                                     
  fMan->AddTask(hit);
    
  fMan->Init();                    
}
