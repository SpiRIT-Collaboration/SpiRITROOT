#include "STPhiEfficiencyTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"
#include "FairFileHeader.h"

#include "TRandom.h"
#include "STVector.hh"
#include "TDatabasePDG.h"
#include "TParameter.h"
#include "TF1.h"
#include "TH2.h"
#include "TPad.h"

#include <cmath>

ClassImp(STPhiEfficiencyTask);

STPhiEfficiencyTask::STPhiEfficiencyTask()
{
  fLogger = FairLogger::GetLogger(); 
  fPhiEff = new TClonesArray("STVectorF");
  for(int i = 0; i < fSupportedPDG.size(); ++i)
    new((*fPhiEff)[i]) STVectorF();
}

STPhiEfficiencyTask::~STPhiEfficiencyTask()
{}

InitStatus STPhiEfficiencyTask::Init()
{
  STAnaParticleDB::FillTDatabasePDG();
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  if(!fEffFilename.empty())
  {
    fLogger -> Info(MESSAGE_ORIGIN, ("Phi Efficiency file is loaded from " + fEffFilename).c_str());
    fEffFile = new TFile(fEffFilename.c_str());
    for(auto pdg : fSupportedPDG)
    {
      fEff[pdg] = (TH2F*) fEffFile -> Get(TString::Format("PhiEfficiency%d", pdg));
      if(!fEff[pdg]) fLogger -> Fatal(MESSAGE_ORIGIN, "Efficiency histogram cannot be loaded. Will ignore efficiency");
      else fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Efficiency histogram of particle %d is loaded", pdg));
    }
    if(auto temp = (TParameter<int>*) fEffFile -> Get("NClus")) fMinNClusters = temp -> GetVal();
    if(auto temp = (TParameter<double>*) fEffFile -> Get("DPOCA")) fMaxDPOCA = temp -> GetVal();
  }
  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");

  ioMan -> Register("PhiEff", "ST", fPhiEff, fIsPersistence);
  return kSUCCESS;
}

void STPhiEfficiencyTask::SetParContainers()
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

void STPhiEfficiencyTask::Exec(Option_t *opt)
{
  auto data = static_cast<STData*>(fData -> At(0));
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    auto& phiEff = static_cast<STVectorF*>(fPhiEff -> At(i)) -> fElements;
    phiEff.clear();
    auto cmVector = static_cast<STVectorVec3*>(fCMVector -> At(i));
    int mult = cmVector -> fElements.size();
    int pdg = fSupportedPDG[i];
    auto TEff = fEff[pdg];

    for(int j = 0; j < mult; ++j)
    {
      TVector2 Q_v1(0, 0), Q_v2(0, 0);
      double weight = 0;
      double eff = 1;
      const auto& vec = cmVector -> fElements[j];
      double theta = vec.Theta();
      double phi = vec.Phi();
      if(TEff && !std::isnan(theta)) eff = TEff -> Interpolate(theta, phi);
      if(!(data -> vaNRowClusters[j] + data -> vaNLayerClusters[j] > fMinNClusters && data -> recodpoca[j].Mag() < fMaxDPOCA)) eff = 0;
      phiEff.push_back(eff);
    }
  }
}

void STPhiEfficiencyTask::LoadPhiEff(const std::string& eff_filename)
{ fEffFilename = eff_filename; }

void STPhiEfficiencyTask::CreatePhiEffFromData(const std::string& ana_filenames, const std::string& out_filename, int nClus, double poca)
{
  STPhiEfficiencyTask::CreatePhiEffFromData(std::vector<std::string>{ana_filenames}, out_filename, nClus, poca);
}

void STPhiEfficiencyTask::CreatePhiEffFromData(const std::vector<std::string>& ana_filenames, const std::string& out_filename, int nClus, double poca)
{
  TChain chain("cbmsim");
  for(const auto& filename : ana_filenames)
    chain.Add(filename.c_str());

  TFile output(out_filename.c_str(), "RECREATE");
  TParameter<int> NClus("NClus", nClus);
  TParameter<double> DPOCA("DPOCA", poca);
  NClus.Write();
  DPOCA.Write();
  const auto& supportedPDG = STAnaParticleDB::GetSupportedPDG();
  for(int i = 0; i < supportedPDG.size(); ++i)
  {
    auto hist_name = TString::Format("PhiEfficiency%d", STAnaParticleDB::GetSupportedPDG()[i]);
    TH2F hist(hist_name, "", 40, 0, 3.15, 100, -3.15, 3.15);
    chain.Project(hist_name, TString::Format("CMVector[%d].fElements.Phi():CMVector[%d].fElements.Theta()", i, i), TString::Format("Prob[%d].fElements*(Prob[%d].fElements > 0.2 && STData[0].vaNRowClusters + STData[0].vaNLayerClusters >%d &&STData[0].recodpoca.Mag() < %f)", i, i, nClus, poca));
    
    // pions has less statistics. Need to rebin just for them
    if(std::abs(supportedPDG[i]) == 211) 
    { 
      hist.RebinX(2);
      hist.RebinY(4);
    }
    // normalize
    hist.Smooth();
    for(int j = 0; j <= hist.GetNbinsX(); ++j)
    {
      // normalize each x-bin
      auto proj = hist.ProjectionY("_px", j, j);
      double max = proj -> GetMaximum();
      max = (max == 0)? 1 : max;
      proj -> Delete();
      for(int k = 0; k <= hist.GetNbinsY(); ++k) 
        hist.SetBinContent(j, k, hist.GetBinContent(j, k)/max);
    }
    output.cd();
    hist.Write();
  }
}

void STPhiEfficiencyTask::SetPersistence(bool val)
{ fIsPersistence = val; }
