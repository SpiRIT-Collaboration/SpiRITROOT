void run_analysis_xml(const std::string& xmlFile="analysisNote.xml", TString fOutName="", bool iter_unfold=false)
{
  /***************************************************************
  * Read xml
  ****************************************************************/
  TDOMParser parser;
  parser.SetValidate(false);
  parser.ParseFile(xmlFile.c_str());
  auto node = parser.GetXMLDocument()->GetRootNode()->GetChildren();
  TXMLNode *TaskNode = nullptr;
  TXMLNode *IONode = nullptr;

  for(auto child = node; child; child = child -> GetNextNode())
    if(child -> GetNodeType() == TXMLNode::kXMLElementNode)
    {
      if(std::strcmp(child -> GetNodeName(), "TaskList") == 0) TaskNode = child;
      if(std::strcmp(child -> GetNodeName(), "IOInfo") == 0) IONode = child;
    }
  STAnalysisFactory factory(TaskNode);

  /***************************************************************
  *  FairRoot setup
  ****************************************************************/
  auto reader = new STConcReaderTask();
  TString fPathToData = reader -> LoadFromXMLNode(IONode).c_str();
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
  int nentries = reader -> GetNEntries();

  std::vector<FairTask*> tasks;
  tasks.push_back(reader);
  tasks.push_back(factory.GetFilterEventTask());
  tasks.push_back(factory.GetPIDTask());
  tasks.push_back(factory.GetTransformFrameTask());
  auto eff = static_cast<STEfficiencyTask*>(factory.GetEfficiencyTask());
  if(eff) eff -> UpdateUnfoldingFile(iter_unfold);
  tasks.push_back(eff);
  tasks.push_back(factory.GetSimpleGraphsTask());
  tasks.push_back(factory.GetERATTask());
  
 
  for(auto task : tasks)
    if(task) run -> AddTask(task);

  run -> Init();
  run -> Run(0, nentries);

  cout << "Log    : " << log << endl;
  cout << "Output : " << out << endl;
}
