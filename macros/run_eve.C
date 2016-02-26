void run_eve
(
 TString name    = "run1278",
 TString parName = "ST.parameters.Cosmic201602.par"
)
{
  TString directory = gSystem -> Getenv("VMCWORKDIR");
  TString input     = directory + "/macros/data/" + name + ".reco.root";
  TString input2    = directory + "/macros/data/" + name + ".mc.root";
  TString input3    = directory + "/macros/data/" + name + ".digi.root";
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
  eve -> SetEveMode("ov");
  eve -> SetInputFile(input);         // Set input file (string)
  eve -> SetParInputFile(parameter);  // Set parameter file (string)
  eve -> SetOutputFile(output);       // Set output file (string)

  if (TString(gSystem -> Which(".", input2)).EqualTo("") == kFALSE)
    eve -> AddFriend(input2);         // Set additional input file (string)
  if (TString(gSystem -> Which(".", input2)).EqualTo("") == kFALSE)
    eve -> AddFriend(input3);

  eve -> SetBackgroundColor(kWhite);  // Set background color (Color_t) 
  eve -> SetGeomFile(geomety);        // Set geometry file (string)
  eve -> SetVolumeTransparency(80);   // Set geometry transparency (integer, 0~100)
  eve -> SetViewerPoint(-0.7, 1.1);   // Set camera angle (theta, phi)



  STEveDrawTask *draw = new STEveDrawTask();

  // Set objects to store while running event display macro.
  draw -> SetObject("mc",         false);
  draw -> SetObject("digi",       false);
  draw -> SetObject("hit",        true);
  draw -> SetObject("hitbox",     false);
  draw -> SetObject("cluster",    true);
  draw -> SetObject("clusterbox", true);
  draw -> SetObject("riemannhit", true);
  draw -> SetObject("curve",      false);
  draw -> SetObject("curvehit",   true);

  // Set objects to render among stored objects. 
  // Rendering is changeable while running macro.
  draw -> SetRendering("mc",         false);
  draw -> SetRendering("digi",       false);
  draw -> SetRendering("hit",        true);
  draw -> SetRendering("hitbox",     false);
  draw -> SetRendering("cluster",    false);
  draw -> SetRendering("clusterbox", false);
  draw -> SetRendering("riemannhit", true);
  draw -> SetRendering("curve",      false);
  draw -> SetRendering("curvehit",   false);

  draw -> SetAttributes("hit", kFullCircle, 1, kGray);


  eve -> AddEveTask(draw);
  eve -> Init();
}
