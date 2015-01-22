eventDisplay_MC()
{
  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *fRun= new FairRunAna();
  fRun->SetInputFile("data/spirit_test.mc.root"); 
  fRun->SetOutputFile("data/spirit_mc_display.root");

  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();

  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  parIo1->open("data/spirit_test.params.root");
  rtdb->setFirstInput(parIo1);
  // ------------------------------------------------------------------------

  FairEventManager   *fMan      = new FairEventManager   ();
  FairMCTracks       *Track     = new FairMCTracks       ("Monte-Carlo Tracks");
  FairMCPointDraw    *RutherfordPoints = new FairMCPointDraw    ("STMCPoint",kBlue , kFullSquare);
                                                     
  fMan->AddTask(Track);
  fMan->AddTask(RutherfordPoints);
    
  fMan->Init();                    
}
