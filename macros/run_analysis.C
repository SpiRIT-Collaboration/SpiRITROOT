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
  TString in1 = fPathToData+fName+".conc.root"; 
  TString out = fPathToData+fOutName+"_ana.root";
  TString log = fPathToData+fOutName+"_ana.log";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  TChain chain("spirit");
  for (const auto &filename : glob(in1.Data())) {
    chain.Add(filename.c_str());
  }
 
  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto reader = new STConcReaderTask();
  reader -> SetChain(&chain);

  //auto PIDCut = new STPIDCutTask();
  //PIDCut -> SetNPitch(5);
  //PIDCut -> SetNYaw(5);
  //PIDCut -> SetCutConditions(15, 20);
  //PIDCut -> SetCutFile("../input/PIDCuts_data.root");
  //PIDCut -> SetCutFile("../input/ParticleCuts.root");

  //auto unfold = new STUnfoldingTask();
  //unfold -> SetMomBins(100, 3700, 50, 25);
  //unfold -> SetThetaBins(0, 1.57, 30, 25);
  //unfold -> LoadMCData("cbmsim", "data/embed_dump/ImQMD_embedCorTriton2/*");
  //unfold -> SetCutConditions(15, 20);

  auto PIDProb = new STPIDProbTask();
  PIDProb -> SetMetaFile("Meta_hpccImQMD.root", true);
  PIDProb -> SetPIDFitFile("PIDSigma_hpcc.root");

  auto transform = new STTransformFrameTask();
  transform -> SetDoRotation(true);
  transform -> SetPersistence(true);
  transform -> SetTargetMass(124);

  std::vector<int> pdgs{2212,1000010020,1000010030,1000020030};
  //auto effFactory = new OrigEfficiencyFactory("/mnt/spirit/analysis/user/tsangc/ForGenie/EfficiencyFactory/MakeDataBase/NewImQMDEff.root");
  //auto effFactory = new EfficiencyFromAnaFactory();
  auto effFactory = new EfficiencyFromConcFactory();
  effFactory -> SetDataBaseForPDG(2212, "data/embed_dump/ImQMD_embedNewProton/*conc*");
  effFactory -> SetDataBaseForPDG(1000010020, "data/embed_dump/ImQMD_embedNewDeuteron/*conc*");
  effFactory -> SetDataBaseForPDG(1000010030, "data/embed_dump/ImQMD_embedNewTriton/*conc*");
  effFactory -> SetDataBaseForPDG(1000020030, "data/embed_dump/ImQMD_embedNewHe3/*conc*");


  auto efficiency = new STEfficiencyTask(effFactory);
  efficiency -> SetParticleList(pdgs);
  for(int pdg : pdgs)
  {
    auto& settings = efficiency -> AccessSettings(pdg);
    settings.NClusters = 15; settings.DPoca = 2000;
    settings.PhiCuts = {{0, 20}, {160, 220}, {320, 360}};
    settings.ThetaMin = 0; settings.ThetaMax = 90; settings.NThetaBins = 20;
    if(pdg == 2212){ settings.NMomBins = 20; settings.MomMin = 100; settings.MomMax = 1500; } 
    if(pdg == 1000010020){ settings.NMomBins = 20; settings.MomMin = 100; settings.MomMax = 3000; } 
    if(pdg == 1000010030){ settings.NMomBins = 20; settings.MomMin = 400; settings.MomMax = 4000; } 
    if(pdg == 1000020030){ settings.NMomBins = 20; settings.MomMin = 200; settings.MomMax = 2000; } 
  }

  run -> AddTask(reader);
  //run -> AddTask(PIDCut);
  run -> AddTask(PIDProb);
  //run -> AddTask(unfold);
  run -> AddTask(transform);
  run -> AddTask(efficiency);

  run -> Init();
  run -> Run(0, 5000);//chain.GetEntries());

  cout << "Log    : " << log << endl;
  cout << "Input : " << in1 << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();

}
