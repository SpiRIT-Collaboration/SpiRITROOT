#include "STEfficiencyTask.hh"
#include "STVector.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TDatabasePDG.h"


ClassImp(STEfficiencyTask);

STEfficiencyTask::STEfficiencyTask()
{ 
  for(int pdg : fSupportedPDG) fEfficiencySettings[pdg] = EfficiencySettings(); 
  fLogger = FairLogger::GetLogger(); 
  fEff = new TClonesArray("STVectorF");
  fEffErr = new TClonesArray("STVectorF");
}

STEfficiencyTask::~STEfficiencyTask()
{}

void STEfficiencyTask::SetFactoriesForParticle(EfficiencyFactory* t_factory, const std::vector<int>& pdgList)
{
  for(const auto& pdg : pdgList) fFactory[pdg] = t_factory;
}

InitStatus STEfficiencyTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  for(auto pdg : fSupportedPDG)
  {
    auto it = fFactory.find(pdg);
    if(it == fFactory.end()) fLogger -> Fatal(MESSAGE_ORIGIN, "Not all particles have been assigend an efficiency factory!");
  }
  if(fFactory.size() > fSupportedPDG.size())
    fLogger -> Info(MESSAGE_ORIGIN, "We received more factoris than the number of types of supported particles. ");

  if(!fUnfoldingFileName.IsNull())
  {
    fLogger -> Info(MESSAGE_ORIGIN, "Setting up unfolding with distribution from " + fUnfoldingFileName);
    fUnfoldingFile = std::unique_ptr<TFile>(new TFile(fUnfoldingFileName, (fUpdateUnfolding)? "UPDATE" : "READ"));
    if(!fUnfoldingFile -> IsOpen())
    {   
      fLogger -> Info(MESSAGE_ORIGIN, "Cannot load unfolding file! Will disable unfolding");
      fUnfoldingFile.reset();
    }
  }
 
  for(int pdg : fSupportedPDG)
  {
    auto factory = fFactory[pdg];
    auto& settings = fEfficiencySettings[pdg];
    fLogger -> Info(MESSAGE_ORIGIN, Form("Loading settings for particle with pdg = %d", pdg));
    if(factory -> IsInCM())
    {
      factory -> SetPtBins(settings.PtMin, settings.PtMax, settings.NPtBins);
      factory -> SetCMzBins(settings.CMzMin, settings.CMzMax, settings.NCMzBins);
      fLogger -> Info(MESSAGE_ORIGIN, Form("Pt is populated from %.2f to %.2f with %d bins", settings.PtMin, settings.PtMax, settings.NPtBins));
      fLogger -> Info(MESSAGE_ORIGIN, Form("CMz is populated from %.2f to %.2f with %d bins", settings.CMzMin, settings.CMzMax, settings.NCMzBins));
      fCMVec = (TClonesArray*) ioMan -> GetObject("CMVector");
      if(!fCMVec) 
        fLogger -> Fatal(MESSAGE_ORIGIN, "You muct add transform task before if you want to use efficiency factory in CM frame.");
    }
    else
    {
      factory -> SetMomBins(settings.MomMin, settings.MomMax, settings.NMomBins);
      factory -> SetThetaBins(settings.ThetaMin, settings.ThetaMax, settings.NThetaBins);
      fLogger -> Info(MESSAGE_ORIGIN, Form("Momentum is populated from %.2f to %.2f with %d bins", settings.MomMin, settings.MomMax, settings.NMomBins));
      fLogger -> Info(MESSAGE_ORIGIN, Form("Theta is populated from %.2f to %.2f with %d bins", settings.ThetaMin, settings.ThetaMax, settings.NThetaBins));

    }
    factory -> SetPhiCut(settings.PhiCuts);
    TString loggerMessage = "Accepting phi range = ";
    for(const auto& range : settings.PhiCuts) loggerMessage += TString::Format("%.2f - %.2f ", range.first, range.second);
    fLogger -> Info(MESSAGE_ORIGIN, loggerMessage + " deg");

    factory -> SetTrackQuality(settings.NClusters, settings.DPoca);
    fLogger -> Info(MESSAGE_ORIGIN, Form("Acceptting N.O. clusters > %d and distance to vertex < %.2f", settings.NClusters, settings.DPoca));
    if(fUnfoldingFile)
      if(auto dist = static_cast<TH2F*>(fUnfoldingFile -> Get(TString::Format("DistUnfolding%d", pdg))))
        factory -> SetUnfoldingDist(dist);
    fEfficiency[pdg] = factory -> FinalizeBins(pdg, true);
  }

  //fPDG = (STVectorI*) ioMan -> GetObject("PDG");
  fData = (TClonesArray*) ioMan -> GetObject("STData");
  // create histogram for unfolding
  for(auto pdg : fSupportedPDG)
  {
    auto& settings = fEfficiencySettings[pdg];
    if(fFactory[pdg] -> IsInCM())
      fDistributionForUnfolding[pdg] = TH2F(TString::Format("PtCMz%d", pdg), TString::Format("Pt vs Pz for %d;Pz (MeV/c);Pt(MeV/c)", pdg),
                                            3*settings.NCMzBins, settings.CMzMin, settings.CMzMax,
                                            3*settings.NPtBins, settings.PtMin, settings.PtMax);
    else
      fDistributionForUnfolding[pdg] = TH2F(TString::Format("PTheta%d", pdg), TString::Format("Lab theta vs lab mom for %d;mom (MeV/c);theta (deg)", pdg),
                                            3*settings.NMomBins, settings.MomMin, settings.MomMax,
                                            3*settings.NThetaBins, settings.ThetaMin, settings.ThetaMax);


  }

  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    new((*fEff)[i]) STVectorF();
    new((*fEffErr)[i]) STVectorF();
  }

  ioMan -> Register("Eff", "ST", fEff, fIsPersistence);
  ioMan -> Register("EffErr", "ST", fEffErr, fIsPersistence);
  fSkip = (STVectorI*) ioMan -> GetObject("Skip");
  
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
  if(fSkip)
    if(fSkip -> fElements[0] == 1) return; // skip == 1 indicates event skip

  auto data = (STData*) fData -> At(0);

  int npart = data -> multiplicity;
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    int ipdg = fSupportedPDG[i];
    auto& TEff = fEfficiency[ipdg];
    const auto& settings = fEfficiencySettings[ipdg];
    auto particle = TDatabasePDG::Instance()->GetParticle(ipdg);

    auto partEff = static_cast<STVectorF*>(fEff -> At(i));
    partEff -> fElements.clear();
    auto partEffErr = static_cast<STVectorF*>(fEffErr -> At(i));
    partEffErr -> fElements.clear();

    auto unfoldIt = fDistributionForUnfolding.find(ipdg);
    STVectorF* probArr = nullptr;
    if(fProb) probArr = static_cast<STVectorF*>(fProb -> At(i));
    bool in_cm = fFactory[ipdg] -> IsInCM();
    for(int part = 0; part < npart; ++part)
    {
      auto& mom = (in_cm)? static_cast<STVectorVec3*>(fCMVec->At(i))->fElements[part] : data -> vaMom[part];
      int nclusters = data -> vaNRowClusters[part] + data -> vaNLayerClusters[part];
      double dpoca = data -> recodpoca[part].Mag();
      double phi = mom.Phi()*TMath::RadToDeg();
      if(phi < 0) phi += 360.;
      bool insidePhi = false;
      for(const auto& cut : settings.PhiCuts)
        if(cut.first < phi && phi < cut.second)
        {
          insidePhi = true;
          break;
        }
      double efficiency = 0;
      double efficiency_err = 0;
      if(nclusters > settings.NClusters 
         && dpoca < settings.DPoca && insidePhi)
      {
         double prob = probArr -> fElements[part];
         if(in_cm)
         {
           double pt = mom.Perp();
           double z = mom.z();
           if(settings.PtMin < pt && pt < settings.PtMax && 
              settings.CMzMin < z && z < settings.CMzMax)
           {
             int bin_id = TEff.FindFixBin(z, pt, phi);
             efficiency = TEff.GetEfficiency(bin_id);
             if(std::abs(ipdg) == 211 && prob > 0) efficiency_err = 0.5*(TEff.GetEfficiencyErrorLow(bin_id) + TEff.GetEfficiencyErrorUp(bin_id));
           }
           if(efficiency > 0.05 && prob > 0.2) unfoldIt -> second.Fill(z, pt, prob/efficiency);
         }
         else
         {
           double momMag = mom.Mag()*fabs((particle->Charge())/3);
           double theta = mom.Theta()*TMath::RadToDeg();
           if(settings.MomMin < momMag && momMag < settings.MomMax) 
           {
             int bin_id = TEff.FindFixBin(momMag, theta, phi);
             efficiency = TEff.GetEfficiency(bin_id);
             if(std::abs(ipdg) == 211 && prob > 0) efficiency_err = 0.5*(TEff.GetEfficiencyErrorLow(bin_id) + TEff.GetEfficiencyErrorUp(bin_id));
           }
           if(efficiency > 0.05 && prob > 0.2) unfoldIt -> second.Fill(momMag, theta, prob/efficiency);
         }
      }
      partEff -> fElements.push_back(efficiency);
      partEffErr -> fElements.push_back(efficiency_err);
    }
  }
}

void STEfficiencyTask::FinishTask()
{
  auto outfile = FairRootManager::Instance() -> GetOutFile();
  outfile -> cd();
  for(const auto& eff : fEfficiency) eff.second.Write(TString::Format("Efficiency%d", eff.first));
  if(fUnfoldingFile && fUpdateUnfolding) fUnfoldingFile -> cd();
  for(const auto& dist : fDistributionForUnfolding) dist.second.Write(TString::Format("DistUnfolding%d", dist.first));
}

void STEfficiencyTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }

