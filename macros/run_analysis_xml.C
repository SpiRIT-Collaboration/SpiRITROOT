void run_analysis_xml(const std::string& xmlFile="analysisNote.xml", TString fOutName="", bool iter_unfold=false, int entries_lim=-1)
{
  /***************************************************************
  * Read xml
  ****************************************************************/
  TDOMParser parser;
  parser.SetValidate(false);
  parser.ParseFile(xmlFile.c_str());
  auto node = parser.GetXMLDocument()->GetRootNode()->GetChildren();
  STAnalysisFactory factory(node);

  /***************************************************************
  *  FairRoot setup
  ****************************************************************/
  auto reader = factory.GetReaderTask();
  int nentries = factory.GetEntries();
  TString fPathToData = factory.GetOutPath();
  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  TString par = spiritroot + "parameters/ST.parameters.par";
  TString geo = spiritroot + "geometry/geomSpiRIT.man.root";
  TString out = fPathToData + fOutName + "_ana.root";
  TString log = fPathToData + fOutName + "_ana.log";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  /*************************************************************
  * Add tasks
  **************************************************************/

  std::vector<FairTask*> tasks;
  tasks.push_back(reader);
  tasks.push_back(factory.GetFilterEventTask());
  tasks.push_back(factory.GetDivideEventTask());

  tasks.push_back(factory.GetPIDTask());
  tasks.push_back(factory.GetTransformFrameTask());
  auto eff = static_cast<STEfficiencyTask*>(factory.GetEfficiencyTask());
  if(eff) eff -> UpdateUnfoldingFile(iter_unfold);
  tasks.push_back(eff);
  tasks.push_back(factory.GetERATTask());
  tasks.push_back(factory.GetReactionPlaneTask());
  tasks.push_back(factory.GetSimpleGraphsTask());

  for(auto task : tasks)
    if(task) run -> AddTask(task);

  run -> Init();
  run -> Run(0, (entries_lim > 0)? entries_lim : nentries);

  cout << "Log    : " << log << endl;
  cout << "Output : " << out << endl;
}
