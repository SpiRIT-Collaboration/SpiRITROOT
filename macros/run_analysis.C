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

void run_analysis_core(TString par, TString geo, TString out,
                       TString log, TChain& chain, int targetMass,
                       TString meta, TString pidFile, bool iterateMeta, 
                       EfficiencyFactory *effFactory, bool targetELoss=true)
{

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetGeomFile(geo);
  run -> SetOutputFile(out);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto reader = new STConcReaderTask();
  reader -> SetChain(&chain);

  //auto filter = new STFilterTask();
  //filter -> SetThetaCut(10);

  //auto PIDCut = new STPIDCutTask();
  //PIDCut -> SetNPitch(1);
  //PIDCut -> SetNYaw(1);
  //PIDCut -> SetCutConditions(15, 15);
  //PIDCut -> SetCutFile("PIDCut_SimSn108.root");

  //auto unfold = new STUnfoldingTask();
  //unfold -> SetMomBins(100, 3700, 50, 25);
  //unfold -> SetThetaBins(0, 1.57, 30, 25);
  //unfold -> LoadMCData("cbmsim", "data/embed_dump/ImQMD_embedCorTriton2/*");
  //unfold -> SetCutConditions(15, 20);
  
  //auto PIDProb = new STPIDMachineLearningTask();
  //PIDProb -> SetChain(&chain);
  //PIDProb -> SetBufferSize(25000);
  //PIDProb -> SetModel("MLForestCut", STAlgorithms::RandomForest);

  auto PIDProb = new STPIDProbTask();
  PIDProb -> SetMetaFile(meta.Data(), iterateMeta);
  PIDProb -> SetPIDFitFile(pidFile.Data());

  auto transform = new STTransformFrameTask();
  transform -> SetDoRotation(true);
  transform -> SetPersistence(true);
  transform -> SetTargetMass(targetMass);
  if(targetELoss)
  {
    transform -> SetTargetThickness(0.8);
    transform -> SetEnergyLossFile((targetMass == 112)? "../parameters/Sn108Sn112.txt": "../parameters/Sn132Sn124.txt");
  }

  auto efficiency = new STEfficiencyTask(effFactory);
  for(int pdg : STAnaParticleDB::SupportedPDG)
  {
    auto& settings = efficiency -> AccessSettings(pdg);
    settings.NClusters = 15; settings.DPoca = 15;
    settings.PhiCuts = {{0, 20}, {160, 220}, {320, 360}};
    settings.ThetaMin = 0; settings.ThetaMax = 90; settings.NThetaBins = 15;
    if(pdg == 2212)      { settings.NMomBins = 15; settings.MomMin = 100; settings.MomMax = 1500; } 
    if(pdg == 1000010020){ settings.NMomBins = 15; settings.MomMin = 200; settings.MomMax = 2200; } 
    if(pdg == 1000010030){ settings.NMomBins = 15; settings.MomMin = 500; settings.MomMax = 4500; } 
    if(pdg == 1000020030){ settings.NMomBins = 15; settings.MomMin = 500; settings.MomMax = 3200; } 
    if(pdg == 1000020040){ settings.NMomBins = 15; settings.MomMin = 1000; settings.MomMax = 4200; } 
    if(pdg == 1000020060){ settings.NMomBins = 10; settings.MomMin = 2000; settings.MomMax = 6000; } 
  }

  run -> AddTask(reader);
  //run -> AddTask(filter);
  //run -> AddTask(PIDCut);
  run -> AddTask(PIDProb);
  //run -> AddTask(unfold);
  if(!iterateMeta)
  {
    run -> AddTask(transform);
    run -> AddTask(efficiency);
  }

  run -> Init();
  run -> Run(0, chain.GetEntries());

  cout << "Log    : " << log << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();

}

void run_analysis
(
  int fStartRunNo,
  int fEndRunNo,
  TString fPathToData,
  int targetMass,
  TString metaFile,
  TString pidFile,
  bool iterateMeta = false
)
{

  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToData.IsNull())
    fPathToData = spiritroot+"macros/data/";
  TString version; {
    TString name = spiritroot + "VERSION.compiled";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  TString par = spiritroot+"parameters/ST.parameters.par";
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root"; 
  TString out = fPathToData + "run";
  if(fEndRunNo > fStartRunNo)
    out += TString::Format("%d_%d", fStartRunNo, fEndRunNo);
  else out += TString::Format("%d", fStartRunNo);
  TString log = out+"_ana.log";
  out += "_ana.root";

  TChain chain("cbmsim");
  std::vector<TString> in;
  for (int runNo = fStartRunNo; runNo <= fEndRunNo; ++runNo) {
    TString sRunNo = TString::Itoa(runNo, 10);
    in.push_back(fPathToData+"run"+sRunNo+"_s*.reco.*trimmed*root");
    std::cout << "Reading from file " << in.back() << std::endl;
    chain.Add(in.back());
  }

  auto effFactory = new EfficiencyFromConcFactory();
  effFactory -> SetDataBaseForPDG(2212, "data/embed_dump/Feb20203/Run2899_embedNewProton/*conc*");
  effFactory -> SetDataBaseForPDG(1000010020, "data/embed_dump/Feb20203/Run2899_embedNewDeuteron/*conc*");
  effFactory -> SetDataBaseForPDG(1000010030, "data/embed_dump/Feb20203/Run2899_embedNewTriton/*conc*");
  effFactory -> SetDataBaseForPDG(1000020030, "data/embed_dump/Feb20203/Run2899_embedNewHe3/*conc*");
  effFactory -> SetDataBaseForPDG(1000020040, "data/embed_dump/Feb20203/Run2899_embedNewHe4/*conc*");
  effFactory -> SetDataBaseForPDG(1000020060, "data/embed_dump/Feb20203/Run2899_embedNewHe6/*conc*");

  run_analysis_core(par, geo, out, log, chain, targetMass, metaFile, pidFile, iterateMeta, effFactory);
}

void run_analysis
(
  TString fName = "he4_lowmom",
  TString fOutName = "",
  TString fPathToData = "",
  int targetMass = 112,
  TString metaFile = "Meta_HalfPD.root",
  TString pidFile = "PIDSigma_HalfPD.root",
  bool iterateMeta = false

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

  TString par = spiritroot+"parameters/ST.parameters.fullmc.par";
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root"; 
  TString in1 = fPathToData+fName+".conc.root"; 
  TString out = fPathToData+fOutName+"_ana.root";
  TString log = fPathToData+fOutName+"_ana.log";

  TChain chain("spirit");
  for (const auto &filename : glob(in1.Data())) {
    chain.Add(filename.c_str());
  }

  auto effFactory = new EfficiencyFromConcFactory();
  effFactory -> SetDataBaseForPDG(2212, "data/embed_dump/Feb20203/SimSn108ProtonCompressed.root");
  effFactory -> SetDataBaseForPDG(1000010020, "data/embed_dump/Feb20203/SimSn108DeuteronCompressed.root");
  effFactory -> SetDataBaseForPDG(1000010030, "data/embed_dump/Feb20203/SimSn108TritonCompressed.root");
  effFactory -> SetDataBaseForPDG(1000020030, "data/embed_dump/Feb20203/SimSn108He3Compressed.root");
  effFactory -> SetDataBaseForPDG(1000020040, "data/embed_dump/Feb20203/SimSn108He4Compressed.root");
  effFactory -> SetDataBaseForPDG(1000020060, "data/embed_dump/Feb20203/Run2899He6Compressed.root");

  run_analysis_core(par, geo, out, log, chain, targetMass, metaFile, pidFile, iterateMeta, effFactory, false);
}


