/**
 * Event display for mc Track
 * This macro will be removed VERY SOON!
 */

void run_eve_mctrack()
{
  FairRunAna *fRun = new FairRunAna();
  fRun -> SetInputFile("data/urqmd_short.mc.root"); 
  fRun -> SetOutputFile("data/urqmd_short.mctrack.root");
  fRun -> SetGeomFile("../geometry/geomSpiRIT.man.root");

  FairEventManager *fMan   = new FairEventManager();
  FairMCTracks     *fTrack = new FairMCTracks("Monte-Carlo Tracks");
  FairMCPointDraw  *fPoint = new FairMCPointDraw("STMCPoint", kBlue, kFullSquare);
                                                     
  fMan -> AddTask(fTrack);
  fMan -> AddTask(fPoint);
  fMan -> Init();                    
}
