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

STEfficiencyTask::STEfficiencyTask(EfficiencyFactory* t_factory)
{ 
  for(int pdg : fSupportedPDG) fEfficiencySettings[pdg] = EfficiencySettings(); 
  fFactory = t_factory;
  fLogger = FairLogger::GetLogger(); 
  fEff = new TClonesArray("STVectorF");
}

STEfficiencyTask::~STEfficiencyTask()
{}

InitStatus STEfficiencyTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  if(!fUnfoldingFileName.IsNull())
  {
    fLogger -> Info(MESSAGE_ORIGIN, "Setting up unfolding with distribution from " + fUnfoldingFileName);
    fUnfoldingFile = std::unique_ptr<TFile>(new TFile(fUnfoldingFileName, "UPDATE"));
    if(!fUnfoldingFile -> IsOpen())
    {   
      fLogger -> Info(MESSAGE_ORIGIN, "Cannot load unfolding file! Will disable unfolding");
      fUnfoldingFile.reset();
    }
  }

  for(int pdg : fSupportedPDG)
  {
    auto& settings = fEfficiencySettings[pdg];
    fLogger -> Info(MESSAGE_ORIGIN, Form("Loading settings for particle with pdg = %d", pdg));
    if(fFactory -> IsInCM())
    {
      fFactory -> SetPtBins(settings.PtMin, settings.PtMax, settings.NPtBins);
      fFactory -> SetCMzBins(settings.CMzMin, settings.CMzMax, settings.NCMzBins);
      fLogger -> Info(MESSAGE_ORIGIN, Form("Pt is populated from %.2f to %.2f with %d bins", settings.PtMin, settings.PtMax, settings.NPtBins));
      fLogger -> Info(MESSAGE_ORIGIN, Form("CMz is populated from %.2f to %.2f with %d bins", settings.CMzMin, settings.CMzMax, settings.NCMzBins));
    }
    else
    {
      fFactory -> SetMomBins(settings.MomMin, settings.MomMax, settings.NMomBins);
      fFactory -> SetThetaBins(settings.ThetaMin, settings.ThetaMax, settings.NThetaBins);
      fLogger -> Info(MESSAGE_ORIGIN, Form("Momentum is populated from %.2f to %.2f with %d bins", settings.MomMin, settings.MomMax, settings.NMomBins));
      fLogger -> Info(MESSAGE_ORIGIN, Form("Theta is populated from %.2f to %.2f with %d bins", settings.ThetaMin, settings.ThetaMax, settings.NThetaBins));

    }
    fFactory -> SetPhiCut(settings.PhiCuts);
    TString loggerMessage = "Accepting phi range = ";
    for(const auto& range : settings.PhiCuts) loggerMessage += TString::Format("%.2f - %.2f ", range.first, range.second);
    fLogger -> Info(MESSAGE_ORIGIN, loggerMessage + " deg");

    fFactory -> SetTrackQuality(settings.NClusters, settings.DPoca);
    fLogger -> Info(MESSAGE_ORIGIN, Form("Acceptting N.O. clusters > %d and distance to vertex < %.2f", settings.NClusters, settings.DPoca));
    if(fUnfoldingFile)
      if(auto dist = static_cast<TH2F*>(fUnfoldingFile -> Get(TString::Format("DistUnfolding%d", pdg))))
        fFactory -> SetUnfoldingDist(dist);
    fEfficiency[pdg] = fFactory -> FinalizeBins(pdg, true);
  }

  for(int i = 0; i < fSupportedPDG.size(); ++i)
    new((*fEff)[i]) STVectorF();

  //fPDG = (STVectorI*) ioMan -> GetObject("PDG");
  fData = (TClonesArray*) ioMan -> GetObject("STData");
  if(fFactory -> IsInCM())
  {
    fCMVec = (TClonesArray*) ioMan -> GetObject("CMVector");
    if(!fCMVec) 
      fLogger -> Fatal(MESSAGE_ORIGIN, "You muct add transform task before if you want to use efficiency factory in CM frame.");
  }

  // create histogram for unfolding
  for(auto pdg : fSupportedPDG)
  {
    auto& settings = fEfficiencySettings[pdg];
    if(fFactory -> IsInCM())
      fDistributionForUnfolding[pdg] = TH2F(TString::Format("PtCMz%d", pdg), TString::Format("Pt vs Pz for %d;Pz (MeV/c);Pt(MeV/c)", pdg),
                                            3*settings.NCMzBins, settings.CMzMin, settings.CMzMax,
                                            3*settings.NPtBins, settings.PtMin, settings.PtMax);
    else
      fDistributionForUnfolding[pdg] = TH2F(TString::Format("PTheta%d", pdg), TString::Format("Lab theta vs lab mom for %d;mom (MeV/c);theta (deg)", pdg),
                                            3*settings.NMomBins, settings.MomMin, settings.MomMax,
                                            3*settings.NThetaBins, settings.ThetaMin, settings.ThetaMax);


  }

  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  ioMan -> Register("Eff", "ST", fEff, fIsPersistence);
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

    auto unfoldIt = fDistributionForUnfolding.find(ipdg);
    STVectorF* probArr = nullptr;
    if(fProb) probArr = static_cast<STVectorF*>(fProb -> At(i));
    for(int part = 0; part < npart; ++part)
    {
      auto& mom = (fFactory -> IsInCM())? static_cast<STVectorVec3*>(fCMVec->At(i))->fElements[part] : data -> vaMom[part];
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
      if(nclusters > settings.NClusters 
         && dpoca < settings.DPoca && insidePhi)
      {
         double prob = probArr -> fElements[part];
         if(fFactory -> IsInCM())
         {
           double pt = mom.Perp();
           double z = mom.z();
           if(settings.PtMin < pt && pt < settings.PtMax && 
              settings.CMzMin < z && z < settings.CMzMax)
             efficiency = TEff.GetEfficiency(TEff.FindFixBin(z, pt, phi));
           if(efficiency > 0.05 && prob > 0.2) unfoldIt -> second.Fill(z, pt, prob/efficiency);
         }
         else
         {
           double momMag = mom.Mag()*(particle->Charge())/3;
           double theta = mom.Theta()*TMath::RadToDeg();
           if(settings.MomMin < momMag && momMag < settings.MomMax) efficiency = TEff.GetEfficiency(TEff.FindFixBin(momMag, theta, phi));
           if(efficiency > 0.05 && prob > 0.2) unfoldIt -> second.Fill(momMag, theta, prob/efficiency);
         }
      }
      partEff -> fElements.push_back(efficiency);
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

