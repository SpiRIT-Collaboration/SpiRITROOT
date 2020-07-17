#include "STSimpleGraphsTask.hh"
#include "STVector.hh"
#include "STAnaParticleDB.hh"
#include "STPIDCutTask.hh"


// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TRandom.h"
#include "TDatabasePDG.h"
#include "TCanvas.h"

ClassImp(STSimpleGraphsTask);

STSimpleGraphsTask::STSimpleGraphsTask() : fEntries(0), fFillRules(fSupportedPDG.size())
{ 
  fLogger = FairLogger::GetLogger(); 
  for(auto pdg : fSupportedPDG) fMinMomForCMInLab[pdg] = 0;

  fMinMomForCMInLab[2212] = 50;
  fMinMomForCMInLab[1000010020] = 300;
  fMinMomForCMInLab[1000010030] = 450;
  fMinMomForCMInLab[1000020030] = 400;
  fMinMomForCMInLab[1000020040] = 400;
}

STSimpleGraphsTask::~STSimpleGraphsTask()
{}

int STSimpleGraphsTask::_ToPitchId(const TVector3& vec)
{
  double pitch = TMath::ATan2(vec.y(), vec.z());
  double dPitch = TMath::Pi()/fNPitches;
  if(std::fabs(pitch) < TMath::Pi()/2.)
    return (pitch + TMath::Pi()/2.)/dPitch;
  else
    return -1;
}

int STSimpleGraphsTask::_ToYawId(const TVector3& vec)
{
  double yaw = TMath::ATan2(vec.x(), vec.z());
  double dYaw = TMath::Pi()/fNYaw;
  if(std::fabs(yaw) < TMath::Pi()/2.)
    return (yaw + TMath::Pi()/2.)/dYaw;
  else
    return -1;
}

void STSimpleGraphsTask::RegisterRuleWithParticle(int pdg, std::function<void(const DataPackage&, const STData&)> rule)
{
  auto it = std::find(fSupportedPDG.begin(), fSupportedPDG.end(), pdg);
  fFillRules[it - fSupportedPDG.begin()].push_back(rule);
}

void STSimpleGraphsTask::RegisterRapidityPlots()
{
  fPlotRapidity = true;
  STAnaParticleDB::FillTDatabasePDG();
  for(auto& pdg : fSupportedPDG)
  {
    auto pname = fParticleName[pdg];
    auto hist_ana = this -> RegisterHistogram<TH2F>(TString::Format("%s_ana", pname.c_str()),
                                                     ";y_{z}/y_{beam Lab};CM P_{t}/A (MeV/c2);",
                                                     100, 0, 2, 60, 0, 1000);
    auto hist_rap = this -> RegisterHistogram<TH1F>(TString::Format("%s_rapHist", pname.c_str()),
                                                     ";2y_{z}/y_{beam Lab};", 100, -2, 2);
    auto hist_pt = this -> RegisterHistogram<TH1F>(TString::Format("%s_ptHist", pname.c_str()),
                                                    "2y_{x}/y_{beam Lab}", 100, -2, 2);
    auto hist_ptFull = this -> RegisterHistogram<TH1F>(TString::Format("%s_ptFullHist", pname.c_str()),
                                                        "2y_{xm}/y_{beam Lab}", 100, -2, 2);
    auto minMom = fMinMomForCMInLab[pdg];
    auto particle = TDatabasePDG::Instance() -> GetParticle(pdg);
    auto pmass = particle -> Mass()/STAnaParticleDB::kAu2Gev;

    this -> RegisterRuleWithParticle(pdg, 
      [hist_ana, hist_rap, hist_pt, hist_ptFull, minMom, pmass, this](const DataPackage& package, const STData& data)
      {
        for(int i = 0; i < data.multiplicity; ++i)
          if(package.eff[i] > 0.05 && package.prob[i] > 0.2)
          {
            if(data.vaMom[i].Mag() > minMom || fIgnoreMinMom)
            {
              hist_ana -> Fill(package.labRapidity[i]/package.beamRapidity[1], 
                               package.cmVector[i].Perp()/pmass, package.weight[i]);
              hist_rap -> Fill(package.fragRapidity[i]/(0.5*package.beamRapidity[1]), 
                               package.weight[i]);
            }
            if(package.cmVector[i].z() > 0 /*package.fragRapidity[i]/(0.5*package.beamRapidity[1]) > -0.2*/)
            {
              double phaseSpaceFactor = 2;
              hist_ptFull -> Fill(package.ptRap[i]/(0.5*package.beamRapidity[1]),
                                  package.weight[i]*phaseSpaceFactor);
              if(fabs(package.fragRapidity[i]/(0.5*package.beamRapidity[1]) - 0.25) < 0.25) 
                hist_pt -> Fill(package.ptRap[i]/(0.5*package.beamRapidity[1]),
                                package.weight[i]*phaseSpaceFactor);
            }
          }
      });
  }
}

void STSimpleGraphsTask::RegisterPIDPlots()
{
  fPlotPID = true;
  for(auto& pdg : fSupportedPDG)
  {
    std::vector<std::vector<TH2F*>> pdg_hists(fNPitches, std::vector<TH2F*>(fNYaw, nullptr));
    for(int i = 0; i < fNPitches; ++i)
      for(int j = 0; j < fNYaw; ++j)
        pdg_hists[i][j] = this -> RegisterHistogram<TH2F>(TString::Format("Pitch%dYaw%dPDG%d", i, j, pdg), "", 
                                                          fMomBins, fMinMom, fMaxMom, 
                                                          fdEdXBins, fMindEdX, fMaxdEdX);

    this -> RegisterRuleWithParticle(pdg,
      [pdg_hists, this](const DataPackage& package, const STData& data)
      {
        for(int i = 0; i < data.multiplicity; ++i)
        {
          int pitchID = this -> _ToPitchId(data.vaMom[i]);
          int yawId = this -> _ToYawId(data.vaMom[i]);
          if(data.vaNRowClusters[i] + data.vaNLayerClusters[i] > fMinNClus && 
             data.recodpoca[i].Mag() < fMaxDPOCA)
            pdg_hists[pitchID][yawId] -> Fill(data.vaMom[i].Mag(), 
                                              data.vadedx[i], package.prob[i]);
        }
      });
  }
}

InitStatus STSimpleGraphsTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  auto db = TDatabasePDG::Instance();

  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  fEff = (TClonesArray*) ioMan -> GetObject("Eff");
  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");
  fLabRapidity = (TClonesArray*) ioMan -> GetObject("LabRapidity");
  fBeamRapidity = (STVectorF*) ioMan -> GetObject("BeamRapidity");
  fFragVelocity = (TClonesArray*) ioMan -> GetObject("FragVelocity");
  fFragRapidity = (TClonesArray*) ioMan -> GetObject("FragRapidity");
  fSkip = (STVectorI*) ioMan -> GetObject("Skip");
  
  return kSUCCESS;
}

void STSimpleGraphsTask::SetParContainers()
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

void STSimpleGraphsTask::Exec(Option_t *opt)
{
  if(fSkip)
    if(fSkip -> fElements[0] == 1) return; // skip == 1 indicates event skip

  auto data = (STData*) fData -> At(0);
  if(fabs(data -> beamEnergy - 270) < 10)
  {
    int npart = data -> multiplicity;
    auto& beamRapidity = fBeamRapidity -> fElements;
     
    for(int i = 0; i < fSupportedPDG.size(); ++i)
    {
      DataPackage dataPackage(static_cast<STVectorF*>(fLabRapidity -> At(i)) -> fElements,
                              static_cast<STVectorF*>(fEff -> At(i)) -> fElements,
                              static_cast<STVectorF*>(fProb -> At(i)) -> fElements,
                              static_cast<STVectorF*>(fFragRapidity -> At(i)) -> fElements,
                              static_cast<STVectorVec3*>(fCMVector -> At(i)) -> fElements,
                              static_cast<STVectorVec3*>(fFragVelocity -> At(i)) -> fElements,
                              beamRapidity);
      for(auto& rule : fFillRules[i]) 
        rule(dataPackage, *data);
    }
    ++fEntries;
  }
}

void STSimpleGraphsTask::FinishTask()
{
  auto outfile = FairRootManager::Instance() -> GetOutFile();
  outfile -> cd();

  auto Set1DUnit = [this](TH1* hist) -> TH1*
  {
    double binSize = hist -> GetXaxis() -> GetBinWidth(0);
    if(hist -> GetDimension() > 1)
      binSize *= hist -> GetYaxis() -> GetBinWidth(0);
    hist -> Scale(1/binSize/fEntries);
    return hist;
  };

  for(auto& hist : f1DHists) 
      Set1DUnit(hist.second) -> Write();

  if(fPlotRapidity)
  {
    std::map<int, TH1D*> ptHists;
    for(int i = 0; i < fSupportedPDG.size(); ++i)
    {
      auto HistName = TString::Format("%s_ana", fParticleName[fSupportedPDG[i]].c_str());
      auto hist = static_cast<TH2F*>(f1DHists[HistName.Data()]);
      auto projx = hist -> ProjectionX(TString::Format("%s_Rap", HistName.Data()));
      auto projy = hist -> ProjectionY(TString::Format("%s_Pt", HistName.Data()),
                                       hist -> GetXaxis() -> FindBin(double(0.5)), 
                                       hist -> GetXaxis() -> FindBin(double(0.75)));
      double dx = hist -> GetXaxis() -> GetBinWidth(0);
      double dy = hist -> GetYaxis() -> GetBinWidth(0);
      projy -> Rebin();
      projy -> Scale(0.5*dx);
      projy -> Write();

      projx -> Rebin();
      projx -> Scale(0.5*dy);
      projx -> Write();
      ptHists[fSupportedPDG[i]] = projy;
    }

    auto THe3 = (TH1D*) ptHists[1000010030] -> Clone("tHe3_ana_Pt");
    THe3 -> Divide(ptHists[1000020030]);
    THe3 -> Write();

    auto pseudoN = (TH1D*) ptHists[2212] -> Clone("PseudoN_ana_Pt");
    pseudoN -> Multiply(THe3);
    pseudoN -> Write();
    
    auto CIP = (TH1D*) ptHists[2212] -> Clone("CIP_ana_Pt");
    auto CIN = (TH1D*) pseudoN -> Clone("CIN_ana_Pt");

    for(auto& key : ptHists)
    {
      auto particle = TDatabasePDG::Instance() -> GetParticle(key.first); 
      int Z = particle -> Charge()/3;
      int A = int(particle -> Mass()/STAnaParticleDB::kAu2Gev + 0.5);
      auto cloned = (TH1D*) key.second -> Clone(TString::Format("%s_ClonedForP", key.second -> GetName()));
      cloned -> Rebin();
      cloned -> Scale(Z/2.);
      CIP -> Add(cloned);

      cloned = (TH1D*) key.second -> Clone(TString::Format("%s_ClonedForN", key.second -> GetName()));
      cloned -> Rebin();
      cloned -> Scale((A - Z)/2.);
      CIN -> Add(cloned); 
    }

    CIP -> Write();
    CIN -> Write();

    CIN -> Divide(CIP);
    CIN -> Write("CINCIP_ana_Pt");
  }

  if(fPlotPID)
  {
    gROOT -> SetBatch(true);
    std::vector<std::vector<TH2F*>> PIDAllHists(fNPitches, std::vector<TH2F*>(fNYaw, nullptr));
    for(int i = 0; i < fSupportedPDG.size(); ++i)
    {
      std::vector<std::vector<TH2F*>> PIDForParticleHists(fNPitches, std::vector<TH2F*>(fNYaw, nullptr));
      for(int pitchId = 0; pitchId < fNPitches; ++pitchId)
        for(int yawId = 0; yawId < fNYaw; ++yawId)
        {
          auto hist = static_cast<TH2F*>(f1DHists[TString::Format("Pitch%dYaw%dPDG%d", pitchId, yawId, fSupportedPDG[i]).Data()]);
          if(PIDAllHists[pitchId][yawId]) PIDAllHists[pitchId][yawId] -> Add(hist);
          else PIDAllHists[pitchId][yawId] = static_cast<TH2F*>(hist -> Clone(TString::Format("Pitch%dYaw%d", pitchId, yawId)));
          PIDForParticleHists[pitchId][yawId] = hist;
        }

      TCanvas c1;
      OrgainzePIDs(&c1, PIDForParticleHists, fNYaw, fNPitches);
      c1.Write(TString::Format("PIDCanvas%d", fSupportedPDG[i]));
    }

    TCanvas c1;
    OrgainzePIDs(&c1, PIDAllHists, fNYaw, fNPitches);
    c1.Write("PIDCanvas");
    gROOT -> SetBatch(false);
  }  
}

void STSimpleGraphsTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }


DataPackage::DataPackage(std::vector<float>& t_labRapidity,
                         std::vector<float>& t_eff,
                         std::vector<float>& t_prob,
                         std::vector<float>& t_fragRapidity,
                         std::vector<TVector3>& t_cmVector,
                         std::vector<TVector3>& t_fragVelocity,
                         std::vector<float>& t_beamRapidity):
  labRapidity(t_labRapidity),
  eff(t_eff),
  prob(t_prob),
  fragRapidity(t_fragRapidity),
  cmVector(t_cmVector),
  fragVelocity(t_fragVelocity),
  beamRapidity(t_beamRapidity)
{
  for(int i = 0; i < t_eff.size(); ++i) 
  {
    weight.push_back(prob[i]/eff[i]);
    float cosphi = cos(gRandom -> Uniform(-TMath::Pi(), TMath::Pi()));
    float perp = fragVelocity[i].Perp();
    float perp_cosphi = cosphi*perp;
    ptRap.push_back(0.5*log((1+perp_cosphi)/(1-perp_cosphi)));
  }
}
