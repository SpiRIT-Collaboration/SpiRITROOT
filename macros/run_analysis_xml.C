#include <glob.h>

std::vector<std::string> glob(const char *pattern) {
    glob_t g;
    glob(pattern, GLOB_TILDE, nullptr, &g); // one should ensure glob returns 0!
    std::vector<std::string> filelist;
    filelist.reserve(g.gl_pathc);
    for (size_t i = 0; i < g.gl_pathc; ++i) {
        filelist.emplace_back(g.gl_pathv[i]);
    }
    globfree(&g);
    return filelist;
} 

void run_analysis_xml(const std::string& xmlFile="analysisNote.xml", TString fOutName="", int NSplit=1, int splitID=0)
{
  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  TString fPathToData = "data/";

  /***************************************************************
  * Read xml
  ****************************************************************/
  TDOMParser parser;
  parser.SetValidate(false);
  parser.ParseFile(xmlFile.c_str());
  auto node = parser.GetXMLDocument()->GetRootNode()->GetChildren();
  TXMLNode *TaskNode = nullptr;
  TChain chain("spirit");

  for(auto child = node; child; child = child -> GetNextNode())
    if(child -> GetNodeType() == TXMLNode::kXMLElementNode)
    {
      if(std::strcmp(child -> GetNodeName(), "TaskList") == 0) TaskNode = child;
      if(std::strcmp(child -> GetNodeName(), "IOInfo") == 0)
      {
        std::string dataType(static_cast<TXMLAttr*>(child -> GetAttributes() -> At(0)) -> GetValue());
        for(auto IOInfo = child -> GetChildren(); IOInfo; IOInfo = IOInfo -> GetNextNode())
          if(std::strcmp(IOInfo -> GetNodeName(), "DataDir") == 0) fPathToData = IOInfo -> GetText(); 

        if(dataType == "Real")
        {
          int start_run, last_run;
          for(auto IOInfo = child -> GetChildren(); IOInfo; IOInfo = IOInfo -> GetNextNode())
          {
            if(std::strcmp(IOInfo -> GetNodeName(), "RunFirst") == 0) start_run = std::atoi(IOInfo -> GetText());
            if(std::strcmp(IOInfo -> GetNodeName(), "RunLast") == 0) last_run = std::atoi(IOInfo -> GetText());
          }
          auto origLevel = gErrorIgnoreLevel;
          gErrorIgnoreLevel = kFatal;
          for(int runNo = start_run; runNo <= last_run; ++runNo)
          {
            TString sRunNo = TString::Itoa(runNo, 10);
            chain.Add(fPathToData+"run"+sRunNo+"_s*.reco.*trimmed*.root/cbmsim");
            chain.Add(fPathToData+"run"+sRunNo+"_s*.reco.*conc*.root/spirit");
            chain.Add(fPathToData+"run"+sRunNo+"_s*.reco.*conc*.root/cbmsim");
            std::cout << "Reading from file " << fPathToData+"run"+sRunNo+"_s*.reco.*root" << std::endl;
          }
          if(chain.GetEntries() == 0) throw std::runtime_error("No entries is being read from the file!");
          gErrorIgnoreLevel = origLevel;
        }
        else if(dataType == "Sim")
        {
          TString inputName;
          for(auto IOInfo = child -> GetChildren(); IOInfo; IOInfo = IOInfo -> GetNextNode())
            if(std::strcmp(IOInfo -> GetNodeName(), "InputName") == 0) inputName = IOInfo -> GetText();
          for(const auto &filename : glob(fPathToData+inputName+".conc.root"))
            chain.Add(filename.c_str());
        }
      }
    }
  STAnalysisFactory factory(TaskNode);

  /***************************************************************
  *  FairRoot setup
  ****************************************************************/

  TString par = spiritroot+"parameters/ST.parameters.par";
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root";
  TString out = fPathToData+fOutName+"_" + TString::Itoa(splitID, 10) + "_ana.root";
  TString log = fPathToData+fOutName+"_" + TString::Itoa(splitID, 10) + "_ana.log";

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

  int nentries = chain.GetEntries();
  int eventsInSplit = nentries / NSplit + 1;
  int eventsStart = splitID*eventsInSplit;
  if(eventsStart + eventsInSplit > nentries) eventsInSplit = nentries - eventsStart;

  auto reader = new STConcReaderTask();
  reader -> SetChain(&chain);
  reader -> SetEventID(eventsStart);
  run -> AddTask(reader);

  std::vector<FairTask*> tasks;
  tasks.push_back(factory.GetFilterEventTask());
  tasks.push_back(factory.GetPIDTask());
  tasks.push_back(factory.GetTransformFrameTask());
  tasks.push_back(factory.GetEfficiencyTask());

  for(auto task : tasks)
    if(task) run -> AddTask(task);

  run -> Init();
  run -> Run(0, eventsInSplit);

  cout << "Log    : " << log << endl;
  cout << "Output : " << out << endl;
}
