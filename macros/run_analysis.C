void run_analysis
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
  TString in1 = fPathToData+fName+".conc.*root"; 
  TString out = fPathToData+fOutName+"_ana.root";
  TString log = fPathToData+fOutName+"_ana.log";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  TChain chain("spirit");
  chain.Add(in1);
 
  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto reader = new STConcReaderTask();
  reader -> SetChain(&chain);

  auto PIDCut = new STPIDCutTask();
  PIDCut -> SetNPitch(5);
  PIDCut -> SetNYaw(5);
  PIDCut -> SetCutFile("../input/ParticleCuts.root");

  auto unfold = new STUnfoldingTask();
  unfold -> SetMomBins(100, 3700, 50, 25);
  unfold -> SetThetaBins(0, 1.57, 30, 25);
  unfold -> LoadMCData("cbmsim", "data/embed_dump/ImQMD_embedCorTriton2/*");
  unfold -> SetCutConditions(15, 20);

  auto transform = new STTransformFrameTask();
  transform -> SetPersistence(true);

  auto efficiency = new STEfficiencyTask();
  efficiency -> EfficiencySettings(15, 20, 0, 100, {{0, 20}, {160, 220}, {320, 360}}, {{0, 360}}, 
                                   "/mnt/spirit/analysis/user/tsangc/ForGenie/EfficiencyFactory/DataBase/ImQMDCorEfficiency.root");

  for(int part = STEfficiencyTask::Particles::Pim; part < STEfficiencyTask::Particles::END; ++part)
    efficiency -> SetThetaBins((STEfficiencyTask::Particles) part, 0, 90, 20);
  efficiency -> SetMomBins(STEfficiencyTask::Particles::Pim, 20, 0, 700);
  efficiency -> SetMomBins(STEfficiencyTask::Particles::Pip, 20, 0, 700);
  efficiency -> SetMomBins(STEfficiencyTask::Particles::Proton, 100, 1500, 20);
  efficiency -> SetMomBins(STEfficiencyTask::Particles::Triton, 400, 4000, 20);
  efficiency -> SetMomBins(STEfficiencyTask::Particles::He3, 200, 2000, 20);

  auto qmdMatch = new STMatchImQMDTask();
  qmdMatch -> SetImQMDFile("../input/imqmd_sn132sn124_RealPionsImpMoreCorTrans.root");

  run -> AddTask(reader);
  run -> AddTask(PIDCut);
  run -> AddTask(unfold);
  run -> AddTask(transform);
  run -> AddTask(efficiency);
  //run -> AddTask(qmdMatch);

  run -> Init();
  run -> Run(0, chain.GetEntries());

  cout << "Log    : " << log << endl;
  cout << "Input : " << in1 << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();

}
