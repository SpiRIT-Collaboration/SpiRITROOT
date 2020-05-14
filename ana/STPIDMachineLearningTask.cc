#include "STPIDMachineLearningTask.hh"
#include "STAnaParticleDB.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

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
  fChain = nullptr; 
  fPDGProb = new TClonesArray("STVectorF", fSupportedPDG.size());
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
  if(endID > fChain -> GetEntries()) endID = fChain -> GetEntries();
  for(int evt = startID; evt < endID; ++evt)
  {
    fChain -> GetEntry(evt);
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

void STPIDMachineLearningTask::TrainModel(const std::string& simulation, const std::string& saveModel, STAlgorithms type, int minClus)
{
  std::string script;
  if(type == STAlgorithms::NeuralNetwork) script = "MachineLearning.py";
  if(type == STAlgorithms::RandomForest) script = "MachineLearningRandomForest.py";
  if(type == STAlgorithms::Voting) script = "MachineLearningVote.py";

  std::string VMCDIR(gSystem -> Getenv("VMCWORKDIR"));
  script = VMCDIR + "/MachineLearning/" + script;
  gSystem -> Exec(("python " + script + " -l " + simulation + " -m " + saveModel/* + " -n " + std::to_string(minClus)*/).c_str());
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
     switch(particlePDG[i])
     {
       case 211: pname = "Pion"; break;
       case 2212: pname = "Proton"; break;
       case 1000010020: pname = "Deuteron"; break;
       case 1000010030: pname = "Triton"; break;
       case 1000020030: pname = "He3"; break;
       case 1000020040: pname = "He4"; break;
       default:
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


