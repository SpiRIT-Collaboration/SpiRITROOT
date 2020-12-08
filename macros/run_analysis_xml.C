void run_analysis_xml(const std::string& xmlFile="analysisNote.xml", TString fOutName="", bool iter_unfold=false, int entries_lim=-1, bool iter_PID=false, int ndivisions=0, int job_id=0)
{
  std::srand(std::time(0));
  gRandom -> SetSeed(std::time(0));
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
  int nentries = reader -> GetNEntries();
  if(entries_lim > 0)
    if(nentries > entries_lim)
      nentries = entries_lim;
  bool use_rand = true;
  if(use_rand && entries_lim > 0) reader -> RandSample(entries_lim);
  if(ndivisions > 1)
  {
    int n_local_entries = int((nentries + 1)/ndivisions);
    int start_id = job_id*n_local_entries;
    if(start_id + n_local_entries > nentries) n_local_entries = nentries - start_id;
    nentries = n_local_entries;
    reader -> SetEventID(start_id);
  }
  TString fPathToData = reader -> GetPathToData();
  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  TString par = spiritroot + "parameters/ST.parameters.par";
  TString geo = spiritroot + "geometry/geomSpiRIT.man.root";
  TString out = fPathToData + fOutName + ((ndivisions <= 1)? Form("_ana.root") : Form("_%d_%d_ana.root", ndivisions, job_id));
  TString log = fPathToData + fOutName + ((ndivisions <= 1)? Form("_ana.log") : Form("_%d_%d_ana.log", ndivisions, job_id));

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
  auto pidTask = factory.GetPIDTask();
  if(auto castedPIDTask = dynamic_cast<STPIDProbTask*>(pidTask))
    castedPIDTask -> SetMetaFileUpdate(iter_PID);
  tasks.push_back(pidTask);
  //auto PIDTask = new STPIDMachineLearningTask();
  //PIDTask -> SetChain((TChain*) ((STConcReaderTask*) reader) -> GetChain());
  //PIDTask -> SetBufferSize(10000);
  //PIDTask -> SetModel("MLModelVotingSn132", STAlgorithms::Voting);
  //tasks.push_back(PIDTask);

  tasks.push_back(factory.GetPiProbTask());
  tasks.push_back(factory.GetDivideEventTask());
  tasks.push_back(factory.GetTransformFrameTask());
  auto eff = factory.GetEfficiencyTask();
  if(eff) eff -> UpdateUnfoldingFile(iter_unfold);
  tasks.push_back(eff);
  tasks.push_back(factory.GetERATTask());
  tasks.push_back(factory.GetFilterEventTask());
  tasks.push_back(factory.GetPhiEfficiencyTask());
  tasks.push_back(factory.GetReactionPlaneTask());
  tasks.push_back(factory.GetSimpleGraphsTask());
  tasks.push_back(factory.GetObsWriterTask());
  tasks.push_back(factory.GetImpactParameterMLTask());

  for(auto task : tasks)
    if(task) run -> AddTask(task);

  run -> Init();
  run -> Run(0, nentries);

  cout << "Log    : " << log << endl;
  cout << "Output : " << out << endl;
}
