#include "STCSVReaderTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include <glob.h>
#include "TXMLAttr.h"
#include "TRandom.h"

ClassImp(STCSVReaderTask);

STCSVReaderTask::STCSVReaderTask(TString filename)
{
  fLogger = FairLogger::GetLogger(); 

  fData = new TClonesArray("STData");
  std::ifstream file(filename);
  if(!file.is_open()) fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot open file " + filename);
  // get rid of header
  std::string line, col;
  std::getline(file, line);
  std::map<std::string, int> name_to_pdg{{"Proton", 2212}, {"Deuteron", 1000010020}, {"Triton", 1000010030}, 
                                         {"He3", 1000020030}, {"He4", 1000020040}};
  while(std::getline(file, line))
  {
    std::stringstream ss(line);
    std::vector<std::string> cols;
    while(std::getline(ss, col, ',')){ cols.push_back(col);}
    try
    { fdEdX.push_back(std::stof(cols[0])); }
    catch(const std::invalid_argument& e)
    { fdEdX.push_back(0); }
    try
    { fParticleList.push_back({name_to_pdg[cols[6]], std::stof(cols[1]), std::stof(cols[2]), std::stof(cols[3]),
                               0., 0., 0.}); }
    catch(const std::invalid_argument& e)
    { fParticleList.push_back({2112, 0, 0, 0, 0., 0., 0.}); }
    try
    { fNClus.push_back(std::stof(cols[5])); }
    catch(const std::invalid_argument& e)
    { fNClus.push_back(0); }
  }
}

STCSVReaderTask::~STCSVReaderTask()
{}

int STCSVReaderTask::GetNEntries()
{ return fParticleList.size(); }

InitStatus STCSVReaderTask::Init()
{
  STAnaParticleDB::FillTDatabasePDG();
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }
 
  new((*fData)[0]) STData();
  ioMan -> Register("STData", "ST", fData, fIsPersistence);
  fTruthPID = new TClonesArray("STVectorI");
  (new((*fTruthPID)[0]) STVectorI) -> fElements.push_back(0);
  ioMan -> Register("TruthPID", "ST", fTruthPID, fIsPersistence);
  return kSUCCESS;
}

void
STCSVReaderTask::SetParContainers()
{
  FairRunAna *run = FairRunAna::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find STDigiPar!");
}

void STCSVReaderTask::Exec(Option_t *opt)
{
  if(fEventID < fParticleList.size())
  {
    fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Event %d", fEventID));
    auto data = static_cast<STData*>(fData -> At(0));
    data -> ResetDefaultWithLength(1);

    const auto& particle = fParticleList[fEventID];
    auto p_info = TDatabasePDG::Instance() -> GetParticle(particle.pdg);
    static_cast<STVectorI*>(fTruthPID -> At(0)) -> fElements[0] = particle.pdg;
    double Z = p_info -> Charge()/3.;

    data -> recodedx[0] = fdEdX[fEventID];
    data -> recodpoca[0].SetXYZ(0, 0, 0);
    data -> recoNRowClusters[0] = 0;
    data -> recoNLayerClusters[0] = fNClus[fEventID]; 
    data -> recoMom[0].SetXYZ(particle.px, particle.py, particle.pz);

    data -> vadedx[0] = fdEdX[fEventID];
    data -> vaMom[0] = data -> recoMom[0];
    data -> vaNRowClusters[0] = data -> recoNRowClusters[0];
    data -> vaNLayerClusters[0] = data -> recoNLayerClusters[0];
  }else fLogger -> Fatal(MESSAGE_ORIGIN, "Event ID exceeds the length of the TChain");
  ++fEventID;
}

void STCSVReaderTask::SetPersistence(Bool_t value)                                              
{ fIsPersistence = value; }

