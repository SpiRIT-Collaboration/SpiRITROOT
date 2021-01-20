#include "STPIDMachineLearningTask.hh"
#include "STAnaParticleDB.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

// ROOT function
#include "TProfile.h"
#include "TGraphSmooth.h"
#include "TCanvas.h"

const std::map<int, std::string> STPIDMachineLearningTask::fMLPDGToName{{211, "Pion"},
                                                                        {2212, "Proton"},
                                                                        {1000010020, "Deuteron"},
                                                                        {1000010030, "Triton"},
                                                                        {1000020030, "He3"},
                                                                        {1000020040, "He4"}};

const std::map<int, int> STPIDMachineLearningTask::fPIDPredictionToPDG{{1, 2212},
                                                                       {2, 1000010020},
                                                                       {3, 1000010030},
                                                                       {4, 1000020030},
                                                                       {5, 1000020040}};
      
const std::vector<std::pair<double, double>> STPIDMachineLearningTask::fValidRange{{0, 600}, 
                                                                                   {200, 1000},
                                                                                   {400, 2000},
                                                                                   {800, 2500},
                                                                                   {800, 1300},
                                                                                   {900,1700}};
 

STTmpFile::STTmpFile(std::ios_base::openmode mode) : buffer{'/','t','m','p','/','S','T','T','m','p','.','X','X','X','X','X','X'},
  fFileName((mkstemp(buffer), buffer)), 
  fFile(buffer, mode)
{}

STTmpFile::~STTmpFile()
{
  fFile.close();
  unlink(buffer);
}

void STTmpFile::ReOpen(std::ios_base::openmode mode)
{
  fFile.close();
  fFile.clear();
  fFile.open(buffer, mode);
}

const std::string STTmpFile::GetFileName() { return fFileName; };

ClassImp(STPIDMachineLearningTask);

STPIDMachineLearningTask::STPIDMachineLearningTask(): fEventID(0)
{
  STAnaParticleDB::EnableChargedParticles();
  fChain = nullptr; 
  fSTData = new STData();

  fLogger = FairLogger::GetLogger(); 
  fAcceptRegion.SetPoint(0,414.268,5427.74);
  fAcceptRegion.SetPoint(1,439.299,4325.7);
  fAcceptRegion.SetPoint(2,476.846,3414.97);
  fAcceptRegion.SetPoint(3,514.393,2384.13);
  fAcceptRegion.SetPoint(4,564.456,1778.36);
  fAcceptRegion.SetPoint(5,664.581,1140.25);
  fAcceptRegion.SetPoint(6,722.987,900.185);
  fAcceptRegion.SetPoint(7,827.284,652.682);
  fAcceptRegion.SetPoint(8,952.441,525.104);
  fAcceptRegion.SetPoint(9,1073.43,422.464);
  fAcceptRegion.SetPoint(10,1215.27,363.146);
  fAcceptRegion.SetPoint(11,1419.69,309.219);
  fAcceptRegion.SetPoint(12,1594.91,276.048);
  fAcceptRegion.SetPoint(13,1841.05,230.651);
  fAcceptRegion.SetPoint(14,2133.08,196.4);
  fAcceptRegion.SetPoint(15,2362.54,172.047);
  fAcceptRegion.SetPoint(16,2671.26,155.05);
  fAcceptRegion.SetPoint(17,5570.71,152.145);
  fAcceptRegion.SetPoint(18,5570.71,18.8218);
  fAcceptRegion.SetPoint(19,5558.2,9.08738);
  fAcceptRegion.SetPoint(20,843.972,9.80151);
  fAcceptRegion.SetPoint(21,731.331,14.3075);
  fAcceptRegion.SetPoint(22,593.659,21.8962);
  fAcceptRegion.SetPoint(23,460.159,30.1995);
  fAcceptRegion.SetPoint(24,314.143,48.9153);
  fAcceptRegion.SetPoint(25,197.33,81.51);
  fAcceptRegion.SetPoint(26,134.752,147.89);
  fAcceptRegion.SetPoint(27,93.0329,260.822);
  fAcceptRegion.SetPoint(28,55.486,438.75);
  fAcceptRegion.SetPoint(29,30.4547,671.463);
  fAcceptRegion.SetPoint(30,17.9391,1430.75);
  fAcceptRegion.SetPoint(31,22.1109,5909.9);
  fAcceptRegion.SetPoint(32,405.924,5966.05);
  fAcceptRegion.SetPoint(33,414.268,5427.74);
}

STPIDMachineLearningTask::~STPIDMachineLearningTask()
{}

InitStatus STPIDMachineLearningTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  if(!fChain)
  {
    fLogger -> Error(MESSAGE_ORIGIN, "TChain is NULL");
    return kERROR;
  }
  
  if(fChain -> GetEntries() == 0)
  {
    fLogger -> Error(MESSAGE_ORIGIN, "The TChain is empty");
    return kERROR;
  }

  if(fSaveModel.empty())
  {
    fLogger -> Error(MESSAGE_ORIGIN, "No machine learning model is selected");
    return kERROR;
  }
  
  std::string MLType;
  if(fMLType == STAlgorithms::NeuralNetwork) MLType = "Neural Network";
  if(fMLType == STAlgorithms::RandomForest) MLType = "Random Forest";
  if(fMLType == STAlgorithms::Voting) MLType = "Voting Algorithm";
  fLogger -> Info(MESSAGE_ORIGIN, (MLType + " is selected").c_str());
  
  if(!fChain -> GetBranch("EvtData"))
    fIsTrimmedFile = true;

  fSupportedPDG = STAnaParticleDB::GetSupportedPDG();
  fPDGProb = new TClonesArray("STVectorF", fSupportedPDG.size());
  ioMan -> Register("Prob", "ST", fPDGProb, fIsPersistence);
  for(int pdg : fSupportedPDG)
    fPDGProbVec[pdg] = static_cast<STVectorF*>(fPDGProb -> ConstructedAt(fPDGProb -> GetEntriesFast()));

  return kSUCCESS;
}

void
STPIDMachineLearningTask::SetParContainers()
{
  FairRunAna *run = FairRunAna::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");
}

void STPIDMachineLearningTask::Exec(Option_t *opt)
{
  if(fEventID % fBufferSize == 0) 
  {
    fLogger -> Info(MESSAGE_ORIGIN, "Asking python for particle PID");
    this -> LoadDataFromPython(fEventID, fEventID + fBufferSize);
  }

  for(auto& ele : fPDGProbVec) ele.second -> fElements.clear();
  auto& pdgVec = fPDGFromPython[fEventID % fBufferSize];
  for(auto pdg : pdgVec)
  {
    for(auto& prob : fPDGProbVec)
    {
      if(prob.first == pdg) prob.second -> fElements.push_back(1);
      else prob.second -> fElements.push_back(0);
    }   
  }
   
  ++fEventID;  
}

void STPIDMachineLearningTask::LoadDataFromPython(int startID, int endID)
{
  // create temprary file for python to read
  STTmpFile input, output;
  fPDGFromPython.clear();

  char *origAddress;
  TBranch *branch;
  if(fIsTrimmedFile) 
  {
    branch = fChain -> GetBranch("STData");
    origAddress = branch -> GetAddress();
    fChain -> SetBranchAddress("STData", &fSTDataArray);
  }
  else
  {
    branch = fChain -> GetBranch("EvtData");
    origAddress = branch -> GetAddress();
    fChain -> SetBranchAddress("EvtData", &fSTData);
  }

  // write dat to buffer
  output.fFile << "dedx\tpx\tpy\tpz\teventid\n";
  std::vector<std::pair<int, int>> excludedParticles;
  if(fTreeIDs.size() == 0)
  {
    if(endID > fChain -> GetEntries()) endID = fChain -> GetEntries();
  }
  else
    if(endID > fTreeIDs.size()) endID = fTreeIDs.size();

  for(int evt = startID; evt < endID; ++evt)
  {
    fChain -> GetEntry((fTreeIDs.size() == 0)? evt : fTreeIDs[evt]);
    if(fIsTrimmedFile) fSTData = (STData*) fSTDataArray->At(0);
    for(int part = 0; part < fSTData->multiplicity; ++part)
    {
      auto vaMom = fSTData->vaMom[part];
      double dedx = fSTData->vadedx[part];
      dedx = (isnan(dedx))? 0 : dedx;
      output.fFile << dedx << "\t" << vaMom.x() << "\t" << vaMom.y() << "\t" << vaMom.z() << "\t" << evt << "\n";

      if(!fAcceptRegion.IsInside(vaMom.Mag(), dedx)) excludedParticles.push_back({evt - startID, part});
    }
  }
  output.fFile << std::flush;

  // as python to analysis the data
  std::string script;
  if(fMLType == STAlgorithms::NeuralNetwork) script = "MachineLearning.py";
  if(fMLType == STAlgorithms::RandomForest) script = "MachineLearningRandomForest.py";
  if(fMLType == STAlgorithms::Voting) script = "MachineLearningVote.py";

  std::string VMCDIR(gSystem -> Getenv("VMCWORKDIR"));
  script = VMCDIR + "/MachineLearning/" + script;
  gSystem -> Exec(("python " + script + " -i " + output.GetFileName() + " -o " + input.GetFileName() + " -m " + fSaveModel).c_str());
  input.ReOpen(std::ios::in);

  // read buffer from python
  std::string line;
  // discard first line as header
  // header is ordered as "evtid type"
  input.fFile >> line >> line;
  int oldevt=-1, evt, type;
  while(input.fFile >> evt >> type)
  {
    if(oldevt != evt) 
    {
      oldevt = evt;
      fPDGFromPython.push_back(std::vector<double>());
    }
    auto& vec = fPDGFromPython.back();
    vec.push_back(type);
  }

  // exclude particles from the list
  for(const auto& id : excludedParticles) fPDGFromPython[id.first][id.second] = -1;
  
  // return branch to original address
  if(fIsTrimmedFile) fChain -> SetBranchAddress("STData", (TClonesArray**) origAddress);
  else fChain -> SetBranchAddress("EvtData", (STData**) origAddress);
  //branch -> SetAddress((void*)origAddress);
}

void STPIDMachineLearningTask::SetPersistence(Bool_t value)                                              
{ fIsPersistence = value; }
void STPIDMachineLearningTask::SetChain(TChain* chain)
{ fChain = chain; }
void STPIDMachineLearningTask::SetBufferSize(int size)
{ fBufferSize = size; }
void STPIDMachineLearningTask::SetModel(const std::string& saveModel, STAlgorithms type)
{ fMLType = type; fSaveModel = saveModel; }

void STPIDMachineLearningTask::dEdXMCDataRatio(const std::string& simulation, const std::string& data_to_match, const std::string& ratio_filename, bool display, int nevent)
{
  // both simulation and data_to_match should be dat file generated from UrQMDWriterTask in light format

  // use Tom's python code to separate PID with KDE
  std::string VMCDIR(gSystem -> Getenv("VMCWORKDIR"));
  std::string script = VMCDIR + "/MachineLearning/PIDSeperator.py";
  // Unsupervised PID of simulated data
  std::string simPID = std::tmpnam(nullptr), dataPID = std::tmpnam(nullptr);
  std::string option = "";
  if(nevent > 0) option = " -n " + std::to_string(nevent);

  std::cout << "Evaluate unsupervised PID on real data" << std::endl;
  gSystem -> Exec(("python " + script + " -i " + data_to_match + " -o " + dataPID + option).c_str());
  std::cout << "Evaluate unsupervised PID on simulated data" << std::endl;
  gSystem -> Exec(("python " + script + " -i " + simulation + " -o " + simPID + option).c_str());

  std::cout << "Reading the output for PID ratio." << std::endl;

  // convert csv output to graph
  auto ReadData = [](const std::string& filename, int type) -> std::vector<std::pair<double, double>>
    {
      std::ifstream file(filename);
      std::string line;
      std::vector<std::pair<double, double>> results;
      // ignore first line
      std::getline(file,line);
      while(std::getline(file,line))
      {
        std::string temp;
        std::vector<std::string> elements;
        std::istringstream ss(line);
        while(std::getline(ss, temp, ',')) elements.push_back(temp);
        if(int(std::stof(elements.back()) + 0.5) == type)
        {
          double px = std::stof(elements[2]);
          double py = std::stof(elements[3]);
          double pz = std::stof(elements[4]);
          results.push_back({std::sqrt(px*px + py*py + pz*pz), std::stof(elements[1])});
        }
      }
      return results;
    };

  // convert TProfile into graph that can be futher smoothed
  auto HistToGraph = [](const TProfile& hist, double xmin, double xmax) -> TGraph
    {
      TGraph graph;
      graph.SetPoint(0, 0, 1);
    
      int bin_min = hist.GetXaxis() -> FindBin(xmin);
      int bin_max = hist.GetXaxis() -> FindBin(xmax);
      for(int i = bin_min; i <= bin_max; ++i) 
        graph.SetPoint(graph.GetN(), hist.GetXaxis() -> GetBinCenter(i), hist.GetBinContent(i));
    
      graph.SetPoint(graph.GetN(), 2*xmax,  1);
      return graph;
    
    };

 
  TFile file(ratio_filename.c_str(), "RECREATE");
  TCanvas *c1 = nullptr;
  for(auto typePDG : fPIDPredictionToPDG)
  { 
    auto particle_data = ReadData(dataPID, typePDG.first);
    auto particle_mc = ReadData(simPID, typePDG.first);

    // data to Histogram for profile
    TH2F PID_MC("PID_MC", "", 50, 0, 4000, 500, 0, 500);
    TH2F PID_data("PID_data", "", 50, 0, 4000, 500, 0, 500);

    for(const auto& content : particle_mc)
      PID_MC.Fill(content.first, content.second);   
    for(const auto& content : particle_data)
      PID_data.Fill(content.first, content.second);

    auto profX_MC = PID_MC.ProfileX();
    auto profX_data = PID_data.ProfileX();
    profX_MC -> Divide(profX_data);

    auto graph = HistToGraph(*profX_MC, fValidRange[typePDG.first].first, fValidRange[typePDG.first].second);
    TGraphSmooth *gs = new TGraphSmooth("normal");
    auto gout = gs -> SmoothSuper(&graph, "", 1);

    file.cd();
    gout -> Write(TString::Format("PIDRatio_%d", typePDG.second));   

    if(display)
    {
      if(!c1)
      {
        c1 = new TCanvas;
        c1 -> Divide(3, 1);
      }
      c1 -> cd(1);
      PID_MC.Draw("colz");
      c1 -> cd(2);
      PID_data.Draw("colz");
      c1 -> cd(3);
      profX_MC -> Draw("hist");
      gout -> Draw("l same");

      c1 -> WaitPrimitive();
    }
  }

  std::remove(dataPID.c_str());
  std::remove(simPID.c_str());
    
}


void STPIDMachineLearningTask::ScaledEdX(STTmpFile& output, const std::string& simulation, const std::string& PID_scale_filename)
{
  std::ifstream file(simulation);
  
  TFile scale_file(PID_scale_filename.c_str());
  std::map<int, TGraph*> ratios;
  
  for(const auto& pdgName : fMLPDGToName)
    ratios[pdgName.first] = (TGraph*) scale_file.Get(TString::Format("PIDRatio_%d", pdgName.first));

  // transfer old data to new
  // header first
  std::string line;
  std::getline(file, line);
  output.fFile << line << "\n";
  
  // scale content
  while(std::getline(file, line))
  {
    std::string temp;
    std::vector<std::string> elements;
    std::stringstream ss(line);
    while(std::getline(ss, temp, ',')) elements.push_back(temp);

    double px = std::stof(elements[1]);
    double py = std::stof(elements[2]);
    double pz = std::stof(elements[3]);
    double pMag = std::sqrt(px*px + py*py + pz*pz);
    for(const auto& pdgName : fMLPDGToName)
      if(pdgName.second == elements.back())
      {
        if(pdgName.first == 211) continue;
        elements[0] = std::to_string(std::stof(elements[0])/ratios[pdgName.first] -> Eval(pMag));
        break;
      }

    output.fFile << elements[0];
    for(int i = 1; i < elements.size(); ++i) output.fFile << "," << elements[i];
    output.fFile << "\n";
  }
  
  output.fFile << std::flush;
}

void STPIDMachineLearningTask::TrainModel(const std::string& simulation, const std::string& saveModel, STAlgorithms type, int minClus, const std::string& scaling_filename)
{
  std::string script;
  if(type == STAlgorithms::NeuralNetwork) script = "MachineLearning.py";
  if(type == STAlgorithms::RandomForest) script = "MachineLearningRandomForest.py";
  if(type == STAlgorithms::Voting) script = "MachineLearningVote.py";

  std::string VMCDIR(gSystem -> Getenv("VMCWORKDIR"));
  script = VMCDIR + "/MachineLearning/" + script;

  if(!scaling_filename.empty())
  {
    STTmpFile output;
    ScaledEdX(output, simulation, scaling_filename);
    gSystem -> Exec(("python " + script + " -l " + output.GetFileName() + " -m " + saveModel + " -n " + std::to_string(minClus)).c_str());
  }
  else gSystem -> Exec(("python " + script + " -l " + simulation + " -m " + saveModel + " -n " + std::to_string(minClus)).c_str());
}

void STPIDMachineLearningTask::ConvertEmbeddingConc(const std::vector<std::string>& embeddingFiles, 
                                                    const std::vector<int>& particlePDG,
                                                    const std::string& simulationFile)
{
  std::ofstream file(simulationFile);
  // print headers
  file << "dEdX,Px,Py,Pz,Charge,NClus,Type\n";
  
  for(int i = 0; i < embeddingFiles.size(); ++i)
  {
     std::string pname;
     try
     {
       pname = fMLPDGToName.at(particlePDG[i]);
     }
     catch(const std::out_of_range& e)
     {
         std::cout << "Particle id is not identified! " << std::endl;
         continue;
     }

     TChain chain("spirit");
     chain.Add(embeddingFiles[i].c_str());
     if(chain.GetEntries() == 0) std::cout << "No data is loaded from " << embeddingFiles[i] << std::endl;
     else 
     {
       std::cout << "Extracting data for " << pname << std::endl;
       int n = chain.Draw("vadedx:vaMom.x():vaMom.y():vaMom.z():vaCharge:vaNRowClusters + vaNLayerClusters","vaEmbedTag", "goff");
       auto v0 = chain.GetVal(0);
       auto v1 = chain.GetVal(1);
       auto v2 = chain.GetVal(2);
       auto v3 = chain.GetVal(3);
       auto v4 = chain.GetVal(4);
       auto v5 = chain.GetVal(5);
       for(int j = 0; j < n; ++j) 
         file << v0[j] << "," << v1[j] << "," << v2[j] << "," 
              << v3[j] << "," << v4[j] << "," << v5[j] << "," << pname << "\n";
     }
  }
}


