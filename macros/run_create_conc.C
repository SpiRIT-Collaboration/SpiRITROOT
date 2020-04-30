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

void run_create_conc(TString fName,
                     Int_t fSplitNo = 0,
                     TString fOutName = "",
                     TString fPathToData = "")
{
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  TString spiritroot = TString(gSystem -> Getenv("VMCWORKDIR"))+"/";
  if (fPathToData.IsNull())
    fPathToData = spiritroot+"macros/data/";
  TString version; {
    TString name = spiritroot + "VERSION.compiled";
    std::ifstream vfile(name);
    vfile >> version;
    vfile.close();
  }

  TString par = spiritroot+"parameters/ST.parameters.fullmc.par";
  TString geo = spiritroot+"geometry/geomSpiRIT.man.root";
  auto inPattern = fPathToData+fName+"_s"+sSplitNo+".reco.*.root";
  auto inList = glob(inPattern);//NewAna.2023.2753c44.root";//develop.1944.33821f0.root";
  if(inList.size() == 0) throw std::runtime_error(std::string(("No pattern matches " + inPattern).Data()));
  TString in1(inList[0]);
  if(inList.size() > 1)
  {
    std::cout << "More than one version of the same run is found in " << fPathToData << std::endl;
    for(const auto& in : inList) std::cout << in << std::endl;
    std::cout << "Only " << in1 << " will be processed" << std::endl;
  }
  TString out = fPathToData+fName+"_s"+sSplitNo+".reco."+version+".conc.root";
  if(!fOutName.IsNull())
    out = fPathToData+fOutName+"_s"+sSplitNo+".reco."+version+".conc.root";

  FairLogger *logger = FairLogger::GetLogger();
  logger -> SetLogToScreen(true);

  FairParAsciiFileIo* parReader = new FairParAsciiFileIo();
  parReader -> open(par);

  FairRunAna* run = new FairRunAna();
  run -> SetInputFile(in1);
  run -> SetOutputFile(out);
  run -> SetGeomFile(geo);
  run -> GetRuntimeDb() -> setSecondInput(parReader);

  auto smallOutput = new STSmallOutputTask();
  //smallOutput -> SetOutputFile(realOut.Data());
  smallOutput -> SetPersistence(true);
  smallOutput -> SetInPlace();

  run -> AddTask(smallOutput);

  run -> Init();
  run -> Run();

  cout << "Input : " << in1 << endl;
  cout << "Output : " << out << endl;

  gApplication -> Terminate();

}

void run_create_conc
(
  Int_t fRunNo = 2894,
  Int_t fSplitNo = 0,
  TString fPathToData = ""
)
{
  TString sRunNo   = TString::Itoa(fRunNo, 10);
  TString sSplitNo = TString::Itoa(fSplitNo, 10);

  run_create_conc("run" + sRunNo, fSplitNo, "", fPathToData);
}


