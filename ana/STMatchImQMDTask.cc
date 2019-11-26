#include <map>
#include <utility>

#include "STMatchImQMDTask.hh"
#include "STVector.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

ClassImp(STMatchImQMDTask);

STMatchImQMDTask::STMatchImQMDTask()
{
  fChain = nullptr; 
  fQMDMom = new TClonesArray("STVectorVec3");
  fQMDPDG = new TClonesArray("STVectorI");
  fQMDDReco = new TClonesArray("STVectorF");
  fLogger = FairLogger::GetLogger(); 
}

STMatchImQMDTask::~STMatchImQMDTask()
{}

InitStatus STMatchImQMDTask::Init()
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

  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fEventID = (TClonesArray*) ioMan -> GetObject("EventID");
  ioMan -> Register("QMDMom", "ST", fQMDMom, fIsPersistence);
  ioMan -> Register("QMDPDG", "ST", fQMDPDG, fIsPersistence);
  ioMan -> Register("QMDDReco", "ST", fQMDDReco, fIsPersistence);
  return kSUCCESS;
}

void
STMatchImQMDTask::SetParContainers()
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

void STMatchImQMDTask::Exec(Option_t *opt)
{
  auto data = (STData*) fData -> At(0);
  fQMDMom -> Delete();
  fQMDPDG -> Delete();
  auto eventID = ((STVectorI*) fEventID -> At(0)) -> fElements[0];
  if(fChain -> GetEntries() > eventID)
  {
    fChain -> GetEntry(eventID);
    int qmd_mult = fQMDPart -> GetEntries();
    int reco_mult = data -> multiplicity;

    auto MatchedMom = new((*fQMDMom)[0]) STVectorVec3();
    auto MatchedPDG = new((*fQMDPDG)[0]) STVectorI();
    auto MatchedDist = new((*fQMDDReco)[0]) STVectorF();
    
    // create distance matrix between momentum
    std::map<std::pair<int, int>, double> distMat;

    // aux variables 
    for(int i = 0; i < qmd_mult; ++i)
      for(int j = 0; j < reco_mult; ++j)
      {
        auto part = (ImQMDParticle*) fQMDPart -> At(i);
        if(part->pz < 0) continue;
        TVector3 qmdMom(part->px, part->py, part->pz);
        qmdMom *= 1000; // GeV to MeV
        auto pdg = part->pdg;
        int Z = (pdg > 2222)? (pdg%10000000)/10000 : 1;
        double dist = ((1./Z)*qmdMom - data -> vaMom[j]).Mag()*Z/qmdMom.Mag();
        distMat[{i, j}] = dist;
      }

    // match each reco particle
    MatchedMom->fElements.resize(reco_mult);
    MatchedPDG->fElements.resize(reco_mult);
    MatchedDist->fElements.resize(reco_mult);

    while(distMat.size() > 0)
    {
      auto min_it = std::min_element(distMat.begin(), distMat.end(), 
                                     [](const std::pair<std::pair<int, int>, double>& p1, const std::pair<std::pair<int, int>, double>& p2){
                                     return p1.second < p2.second; });
      auto id = min_it -> first;
      int idQMDMin = id.first;
      int idRecoMin = id.second;
      auto part = (ImQMDParticle*) fQMDPart -> At(idQMDMin);
      auto pdg = part->pdg;
      MatchedPDG->fElements[idRecoMin] = pdg;
      int Z = (pdg > 2222)? (pdg%10000000)/10000 : 1;
      MatchedMom->fElements[idRecoMin].SetXYZ(part->px/Z, part->py/Z, part->pz/Z);
      MatchedDist->fElements[idRecoMin] = min_it -> second;
      for(int i = 0; i < reco_mult; ++i) distMat.erase({idQMDMin, i});
      for(int i = 0; i < qmd_mult; ++i) distMat.erase({i, idRecoMin});
    }

  }else fLogger -> Fatal(MESSAGE_ORIGIN, "Event ID exceeds the length of the given ImQMD file");
}

void STMatchImQMDTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STMatchImQMDTask::SetImQMDFile(const std::string& t_qmdFile, const std::string& t_qmdName)
{ 
  fLogger -> Info(MESSAGE_ORIGIN, ("Matching simulated result with ImQMD data from file " + t_qmdFile).c_str());
  fChain = new TChain(t_qmdName.c_str());
  fChain -> Add(t_qmdFile.c_str());

  fChain ->SetBranchAddress("ImQMD", &fQMDPart);
}
