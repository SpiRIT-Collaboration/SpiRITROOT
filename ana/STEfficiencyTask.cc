#include "STEfficiencyTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

ClassImp(STEfficiencyTask);

STEfficiencyTask::STEfficiencyTask(EfficiencyFactory* t_factory)
{ 
  fFactory = t_factory;
  fLogger = FairLogger::GetLogger(); 
  fEff = new STVectorF();
}

STEfficiencyTask::~STEfficiencyTask()
{}

void STEfficiencyTask::SetParticleList(const std::vector<int>& t_plist)
{ 
  fSupportedPDG = t_plist; 
  for(int pdg : fSupportedPDG) fEfficiencySettings[pdg] = EfficiencySettings(); 
}

InitStatus STEfficiencyTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  for(int pdg : fSupportedPDG)
  {
    auto& settings = fEfficiencySettings[pdg];
    fFactory -> SetMomBins(settings.MomMin, settings.MomMax, settings.NMomBins);
    fFactory -> SetThetaBins(settings.ThetaMin, settings.ThetaMax, settings.NThetaBins);
    fFactory -> SetPhiCut(settings.PhiCuts);
    fFactory -> SetTrackQuality(settings.NClusters, settings.DPoca);
    fEfficiency[pdg] = fFactory -> FinalizeBins(pdg, true);
  }

  fPDG = (STVectorI*) ioMan -> GetObject("PDG");
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
  fEff -> fElements.clear();
  auto data = (STData*) fData -> At(0);

  int npart = data -> multiplicity;
  for(int part = 0; part < npart; ++part)
  {
    auto& mom = data -> vaMom[part];
    int nclusters = data -> vaNRowClusters[part] + data -> vaNLayerClusters[part];
    double dpoca = data -> recodpoca[part].Mag();
    int ipdg = fPDG -> fElements[part];
    double efficiency = 0;
    if(ipdg != 0)
    {
      auto it = fEfficiency.find(ipdg);
      if(it != fEfficiency.end())
      {
        const auto& settings = fEfficiencySettings[ipdg];
        double phi = mom.Phi()*TMath::RadToDeg();
        if(phi < 0) phi += 360.;
        bool insidePhi = false;
        for(const auto& cut : settings.PhiCuts)
          if(cut.first < phi && phi < cut.second)
          {
            insidePhi = true;
            break;
          }

        if(nclusters > settings.NClusters && dpoca < settings.DPoca && insidePhi)
        {
          auto& TEff = it -> second;
          efficiency = TEff.GetEfficiency(TEff.FindFixBin(mom.Mag(), mom.Theta()*TMath::RadToDeg(), phi));
        }
      }
    }
    fEff -> fElements.push_back(efficiency);
  }
}

void STEfficiencyTask::FinishTask()
{
  auto outfile = FairRootManager::Instance() -> GetOutFile();
  outfile -> cd();
  for(const auto& eff : fEfficiency) eff.second.Write(TString::Format("Efficiency%d", eff.first));
}

void STEfficiencyTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }

