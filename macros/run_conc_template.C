void run_conc_template
(
  TString fName = "he4_lowmom",
  TString fOutName = "",
  TString fParameterFile = "ST.parameters.fullmc.par",
  TString fPathToData = ""
)
{
  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToData.IsNull())
    fPathToData = spiritroot+"macros/data/";
  TString version; {
    TString name = spiritroot + "VERSION";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  if(fOutName.IsNull()) fOutName = fName;

  TString par = spiritroot+"parameters/"+fParameterFile;
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root"; 
  TString in1 = fPathToData+fName+".conc.root"; 
  TString out = fPathToData+fOutName+"_ana.root";
  TString log = fPathToData+fOutName+"_ana.log";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  TChain chain("cbmsim");
  chain.Add(in1);
 
  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto reader = new STConcReaderTask();
  reader -> SetChain(&chain);

  run -> AddTask(reader);

  run -> Init();
  run -> Run(0, chain.GetEntries());

  cout << "Log    : " << log << endl;
  cout << "Input : " << in1 << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();

}
