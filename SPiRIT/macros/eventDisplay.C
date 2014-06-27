eventDisplay()
{
  // -----   Reconstruction run   -------------------------------------------
  FairRunAna *fRun= new FairRunAna();
  fRun->SetInputFile("data/test.mc_youngstest.root"); 
  fRun->SetOutputFile("data/test.root");

  // -----  Parameter database   --------------------------------------------
  FairRuntimeDb* rtdb = fRun->GetRuntimeDb();

  FairParRootFileIo* parIo1 = new FairParRootFileIo();
  parIo1->open("data/params_youngstest.root");
  rtdb->setFirstInput(parIo1);
  // ------------------------------------------------------------------------

  FairEventManager   *fMan      = new FairEventManager   ();
  FairMCTracks       *Track     = new FairMCTracks       ("Monte-Carlo Tracks");
  FairMCPointDraw    *RutherfordPoints = new FairMCPointDraw    ("SPiRITPoint",kBlue , kFullSquare);
                                                     
  fMan->AddTask(Track);
  fMan->AddTask(RutherfordPoints);
    
  fMan->Init();                    
}
