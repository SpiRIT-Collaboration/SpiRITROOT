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
  //std::cout << buffer << std::endl;
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
    branch -> SetAddress(&fSTDataArray);
  }
  else
  {
    branch = fChain -> GetBranch("EvtData");
    origAddress = branch -> GetAddress();
    branch -> SetAddress(&fSTData);
  }

  // write dat to buffer
  output.fFile << "dedx\tpx\tpy\tpz\teventid\n";
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
    }
  }
  output.fFile << std::flush;

  // as python to analysis the data
  std::string script;
  if(fMLType == STAlgorithms::NeuralNetwork) script = "MachineLearning.py";
  if(fMLType == STAlgorithms::RandomForest) script = "MachineLearningRandomForest.py";

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

  std::string VMCDIR(gSystem -> Getenv("VMCWORKDIR"));
  script = VMCDIR + "/MachineLearning/" + script;
  gSystem -> Exec(("python " + script + " -l " + simulation + " -m " + saveModel + " -n " + std::to_string(minClus)).c_str());
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


