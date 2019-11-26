#include "STEfficiencyTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

ClassImp(STEfficiencyTask);

STEfficiencyTask::STEfficiencyTask()
{ 
  fLogger = FairLogger::GetLogger(); 
  fEff = new TClonesArray("STVectorF");
}

STEfficiencyTask::~STEfficiencyTask()
{}

void STEfficiencyTask::EfficiencySettings(int t_num_clusters,
                                          double t_dist_2_vert,
                                          int t_min_mult,
                                          int t_max_mult,
                                          const std::vector<std::pair<double, double>>& t_phi_cut,
                                          const std::vector<std::pair<double, double>>& t_phi_range,
                                          const std::string& t_efficiency_db,
                                          const std::string& t_cut_db,
                                          double t_cluster_ratio)
{
  for(auto& pname : fParticleNameEff)//int i = Particles::Pim; i < Particles::END; ++i)
    fEfficiencyFactory.emplace_back(new EfficiencyFactory(t_num_clusters, t_dist_2_vert,
                                                          t_min_mult, t_max_mult,
                                                          t_phi_cut, pname, t_phi_range,
                                                          t_efficiency_db, t_cut_db, t_cluster_ratio));
}

void STEfficiencyTask::SetMomBins(Particles part, double t_min, double t_max, int t_bins)
{  fEfficiencyFactory[part] -> SetMomBins(t_min, t_max, t_bins); }

void STEfficiencyTask::SetThetaBins(Particles part, double t_min, double t_max, int t_bins)
{  fEfficiencyFactory[part] -> SetThetaBins(t_min, t_max, t_bins); }

InitStatus STEfficiencyTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  for(auto& factory : fEfficiencyFactory)
    fEfficiency.push_back(factory -> FinalizeBins(true, false, false, false));

  fPDG = (TClonesArray*) ioMan -> GetObject("PDG");
  fData = (TClonesArray*) ioMan -> GetObject("STData");

  ioMan -> Register("Eff", "ST", fEff, fIsPersistence);
  return kSUCCESS;
}

void STEfficiencyTask::SetParContainers()
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

void STEfficiencyTask::Exec(Option_t *opt)
{
  fEff -> Delete();
  auto data = (STData*) fData -> At(0);
  auto pdg = (STVectorI*) fPDG -> At(0);
  auto EffAry = new((*fEff)[0]) STVectorF();

  int npart = data -> multiplicity;
  for(int part = 0; part < npart; ++part)
  {
    auto& mom = data -> vaMom[part];
    int nclusters = data -> vaNRowClusters[part] + data -> vaNLayerClusters[part];
    double dpoca = data -> recodpoca[part].Mag();
    int ipdg = pdg -> fElements[part];
    double efficiency = 0;
    if(ipdg != 0)
    {
      auto it = std::find(fSupportedPDG.begin(), fSupportedPDG.end(), ipdg);
      if(it != fSupportedPDG.end())
      {
        int id = it - fSupportedPDG.begin();
        double phi = mom.Phi()*TMath::RadToDeg();
        if(phi < 0) phi += 360.;
        if(nclusters > fEfficiencyFactory[id] -> num_clusters_
           && dpoca < fEfficiencyFactory[id] -> dist_2_vert_
           && fEfficiencyFactory[id]->_InsidePhi(phi, fEfficiencyFactory[id]->phi_cut_))
        {
          auto& TEff = fEfficiency[it - fSupportedPDG.begin()];
          efficiency = TEff.GetEfficiency(TEff.FindFixBin(mom.Mag(), mom.Theta()*TMath::RadToDeg(), phi));
        }
      }
    }
    EffAry -> fElements.push_back(efficiency);
  }
}

void STEfficiencyTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }

