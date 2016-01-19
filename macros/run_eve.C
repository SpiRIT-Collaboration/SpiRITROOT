void run_eve
(
 TString name    = "urqmd_short",
 TString parName = "ST.parameters.par"
)
{
  TString directory = gSystem -> Getenv("VMCWORKDIR");
  TString input     = directory + "/macros/data/" + name + ".reco.root";
  TString input2    = directory + "/macros/data/" + name + ".digi.root";
  TString output    = directory + "/macros/data/" + name + ".eve.root";
  TString parameter = directory + "/parameters/"  + parName;
  TString geomety   = directory + "/geometry/geomSpiRIT.man.root";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);
  logger -> SetLogScreenLevel("debug");



  STEveManager *eve = new STEveManager();

  // Set event display mode (string) 
  // - 3d  : Activate 3D view tab.
  // - ov  : Activate overview tab (include pad-plane and adc-dist.)
  // - all : Activate Both 3D and overview tab.
  eve -> SetEveMode("all");

  eve -> SetInputFile(input);         // Set input file (string)
  eve -> AddFriend(input2);           // Set additional input file (string)
  eve -> SetParInputFile(parameter);  // Set parameter file (string)
  eve -> SetOutputFile(output);       // Set output file (string)

  eve -> SetBackgroundColor(kWhite);  // Set background color (Color_t) 
  eve -> SetGeomFile(geomety);        // Set geometry file (string)
  eve -> SetVolumeTransparency(80);   // Set geometry transparency (integer, 0~100)
  eve -> SetViewerPoint(-0.7, 1.1);   // Set camera angle (theta, phi)



  STEveDrawTask *draw = new STEveDrawTask();

  // Set objects to store while running event display macro.
  draw -> SetObject("mc",         false);  // MC hit point
  draw -> SetObject("digi",       false);  // Digi hit point
  draw -> SetObject("hit",         true);  // Reconstructed hit point
  draw -> SetObject("hitbox",     false);  // Hit point in colored(proportional to charge) box
  draw -> SetObject("cluster",    false);  // Hit cluster point
  draw -> SetObject("clusterbox", false);  // Hit cluster point in box with size of sigmas
  draw -> SetObject("riemannhit",  true);  // Riemann track hit point, distinguished by colors
  draw -> SetObject("linear",      true);  // Linear track line
  draw -> SetObject("linearhit",  false);  // Linear track point, distinguished by colors

  // Set objects to render among stored objects. 
  // Rendering is changeable while running macro.
  draw -> SetRendering("mc",         false);
  draw -> SetRendering("digi",       false);
  draw -> SetRendering("hit",         true);
  draw -> SetRendering("hitbox",     false);
  draw -> SetRendering("cluster",    false);
  draw -> SetRendering("clusterbox", false);
  draw -> SetRendering("riemannhit",  true);
  draw -> SetRendering("linear",     false);
  draw -> SetRendering("linearhit",  false);



  eve -> AddEveTask(draw);
  eve -> Init();
}
