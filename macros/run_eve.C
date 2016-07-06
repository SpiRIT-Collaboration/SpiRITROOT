void run_eve
(
  TString name = "run3000_s0", 
  TString parname = "ST.parameters.Commissioning_201604.par")
{
  TString directory = gSystem -> Getenv("VMCWORKDIR");
  TString input     = directory + "/macros/data/" + name + ".reco.root";
  TString output    = directory + "/macros/data/" + name + ".eve.root";
  TString parameter = directory + "/parameters/"  + parname;
  TString geomety   = directory + "/geometry/geomSpiRIT.man.root";

  STEveManager *eve = new STEveManager();
  eve -> SetInputFile(input);         // Set input file (string)
  eve -> SetParInputFile(parameter);  // Set parameter file (string)
  eve -> SetOutputFile(output);       // Set output file (string)
  eve -> SetBackgroundColor(kWhite);  // Set background color (Color_t) 
  eve -> SetGeomFile(geomety);        // Set geometry file (string)
  eve -> SetVolumeTransparency(80);   // Set geometry transparency (integer, 0~100)
  eve -> SetViewerPoint(-0.7, 1.1);   // Set camera angle (theta, phi)

  STEveDrawTask *draw = new STEveDrawTask();
  draw -> SetRendering("mc",         false);
  draw -> SetRendering("digi",       false);
  draw -> SetRendering("hit",        false);
  draw -> SetRendering("hitbox",     false);
  draw -> SetRendering("cluster",    true);
  draw -> SetRendering("helix",      true);
  draw -> SetRendering("helixhit",   true);
  draw -> SetRendering("recotrack",  true);

  draw -> SetAttributes("hit", kFullCircle, 1, kGray);

  eve -> AddEveTask(draw);
  eve -> Init();
}
