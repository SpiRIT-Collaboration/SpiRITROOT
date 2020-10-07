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

#include "TXMLNode.h"
#include "TDOMParser.h"
#include "TRandom.h"
#include "TDatabasePDG.h"
#include "TCanvas.h"
#include "TParameter.h"
#include "TF1.h"

#include <cstring>
#include <cmath>

const std::map<int, std::string> STSimpleGraphsTask::fParticleName{{2212, "p"}, 
                                                                   {1000010020, "d"}, 
                                                                   {1000010030, "t"}, 
                                                                   {1000020030, "He3"}, 
                                                                   {1000020040, "He4"}, 
                                                                   {1000020060, "He6"},
                                                                   {211, "pip"},
                                                                   {-211, "pim"}};



DataPackage::DataPackage() : fTCArrList(ARREND, nullptr), fVecList(VECEND, nullptr)
{}

void DataPackage::CheckEmptyElements(int n_particle_type)
{
  for(const auto& def_val : fDefaultValues)
  {
    if(!fTCArrList[def_val.first])
    {
      fTCArrList[def_val.first] = new TClonesArray("STVectorF");   
      for(int i = 0; i < n_particle_type; ++i) 
      {
        auto eff = new((*fTCArrList[def_val.first])[i]) STVectorF();
        eff -> fElements.resize(50, def_val.second);
      }
    }
  }
}

void DataPackage::UpdateData(int part_id)
{
  fPartID = part_id;
  // first check if efficiency is empty
  // if so fill everything with one
  auto data = static_cast<STData*>(fTCArrList[DATA] -> At(0));
  for(const auto& def_val : fDefaultValues)
  {
    auto& val = static_cast<STVectorF*>(fTCArrList[def_val.first] -> At(fPartID)) -> fElements;
    if(val.size() < data -> multiplicity)
      val.resize(data -> multiplicity, def_val.second);  
  }

  fWeight.clear();
  fPtxRap.clear();
  for(int i = 0; i < data -> multiplicity; ++i)
  {
    fWeight.push_back(Prob(i)/Eff(i));
    float cosphi = cos(gRandom -> Uniform(-TMath::Pi(), TMath::Pi()));
    float perp = FragVelocity(i).Perp();
    float perp_cosphi = cosphi*perp;
    fPtxRap.push_back(0.5*log((1+perp_cosphi)/(1-perp_cosphi)));
  }

}

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
  fMinMomForCMInLab[211] = 0;
  fMinMomForCMInLab[-211] = 0;

}

STSimpleGraphsTask::~STSimpleGraphsTask()
{}


InitStatus STSimpleGraphsTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  auto db = TDatabasePDG::Instance();

  fDataPackage.fTCArrList[DataPackage::DATA] = (TClonesArray*) ioMan -> GetObject("STData");
  fDataPackage.fTCArrList[DataPackage::PROB] = (TClonesArray*) ioMan -> GetObject("Prob");
  fDataPackage.fTCArrList[DataPackage::SD] = (TClonesArray*) ioMan -> GetObject("SD");
  fDataPackage.fTCArrList[DataPackage::EFF] = (TClonesArray*) ioMan -> GetObject("Eff");
  fDataPackage.fTCArrList[DataPackage::EFFERR] = (TClonesArray*) ioMan -> GetObject("EffErr");
  fDataPackage.fTCArrList[DataPackage::CMVECTOR] = (TClonesArray*) ioMan -> GetObject("CMVector");
  fDataPackage.fTCArrList[DataPackage::LABRAPIDITY] = (TClonesArray*) ioMan -> GetObject("LabRapidity");
  fDataPackage.fTCArrList[DataPackage::FRAGVELOCITY] = (TClonesArray*) ioMan -> GetObject("FragVelocity");
  fDataPackage.fTCArrList[DataPackage::FRAGRAPIDITY] = (TClonesArray*) ioMan -> GetObject("FragRapidity");
  fDataPackage.fTCArrList[DataPackage::PHIEFF] = (TClonesArray*) ioMan -> GetObject("PhiEff");
  fDataPackage.fTCArrList[DataPackage::V1RPANGLE] = (TClonesArray*) ioMan -> GetObject("V1RPAngle");
  fDataPackage.fTCArrList[DataPackage::V2RPANGLE] = (TClonesArray*) ioMan -> GetObject("V2RPAngle");

  fDataPackage.fVecList[DataPackage::BEAMRAPIDITY] = (STVectorF*) ioMan -> GetObject("BeamRapidity");
  fDataPackage.fVecList[DataPackage::BEAMMOM] = (STVectorF*) ioMan -> GetObject("BeamMom");
  fSkip = (STVectorI*) ioMan -> GetObject("Skip");

  for(const auto type : fTypeToDiscard) fDataPackage.fTCArrList[type] = nullptr;
  fDataPackage.CheckEmptyElements(fSupportedPDG.size()); // sometimes efficiency maybe empty. In that case efficiency will be auto-filled by ones

  // if efficiency is empty, fill it with ones 
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

  //if(fabs(fDataPackage.Data().beamEnergy - 270) < 10)
  {
    for(int i = 0; i < fSupportedPDG.size(); ++i)
    {
      fDataPackage.UpdateData(i);
      for(auto& rule : fFillRules[i]) rule(fDataPackage);
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

  for(auto it = f1DHists.begin(); it != f1DHists.end(); ++it)
    if(fNormalize[it -> first])
      Set1DUnit(it -> second) -> Write();
    else
      it -> second -> Write();      

  for(auto rule : fFinishTaskRule)
    rule();

  TParameter<int> entries("Entries", fEntries);
  entries.Write();
}

void STSimpleGraphsTask::RemoveParticleMin()
{ for(auto& val : fMinMomForCMInLab) val.second = -9999; }
void STSimpleGraphsTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STSimpleGraphsTask::RegisterFinishTaskRule(std::function<void()> rule)                        { fFinishTaskRule.push_back(rule); }

/**************************************
* Below are rules to draw histograms
* Should be independent of the above 
**************************************/

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

void STSimpleGraphsTask::RegisterRuleWithParticle(int pdg, std::function<void(const DataPackage&)> rule)
{
  auto it = std::find(fSupportedPDG.begin(), fSupportedPDG.end(), pdg);
  fFillRules[it - fSupportedPDG.begin()].push_back(rule);
}

void STSimpleGraphsTask::RegisterRapidityPlots()
{
  fPlotRapidity = true;
  STAnaParticleDB::FillTDatabasePDG();
  std::map<int, TH2F*> ana_hists;
  for(auto& pdg : fSupportedPDG)
  {
    auto pname = fParticleName.at(pdg);
    auto hist_ana = this -> RegisterHistogram<TH2F>(true, TString::Format("%s_ana", pname.c_str()),
                                                     ";2y_{z}/y_{beam Lab};CM P_{t}/A (MeV/c2);",
                                                     200, -4, 4, 40, 0, 1000);
    auto hist_rap = this -> RegisterHistogram<TH1F>(true, TString::Format("%s_rapHist", pname.c_str()),
                                                     ";2y_{z}/y_{beam Lab};", 200, -4, 4);
    auto hist_pt = this -> RegisterHistogram<TH1F>(true, TString::Format("%s_ptHist", pname.c_str()),
                                                    "2y_{x}/y_{beam Lab}", 200, -4, 4);
    auto hist_ptFull = this -> RegisterHistogram<TH1F>(true, TString::Format("%s_ptFullHist", pname.c_str()),
                                                        "2y_{xm}/y_{beam Lab}", 200, -4, 4);
    auto particle = TDatabasePDG::Instance() -> GetParticle(pdg);
    auto pmass = int(particle -> Mass()/STAnaParticleDB::kAu2Gev);
    pmass = (pmass == 0)? 1 : std::abs(pmass);

    this -> RegisterRuleWithParticle(pdg, 
      [this, hist_ana, hist_rap, hist_pt, hist_ptFull, pmass, pdg](const DataPackage& package)
      {
        const auto& data = package.Data();
        for(int i = 0; i < data.multiplicity; ++i)
          if(package.Eff(i) > fEffThresholdForRap && package.Prob(i) > fProbThresholdForRap && 
             std::fabs(package.StdDev(i)) < fSDThresholdForRap)
          {
            double y0 = package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);
            auto minMom = fMinMomForCMInLab[pdg];
            if(data.vaMom[i].Mag() > minMom)
            {
              hist_ana -> Fill(y0, package.CMVector(i).Perp()/pmass, package.Weight(i));
              if(package.CMVector(i).Perp()/pmass/package.BeamMom()[0] > fPtThresholdForRap) hist_rap -> Fill(y0, package.Weight(i));
            }
            if(package.CMVector(i).z() > 0 /*package.fragRapidity(i)/(0.5*package.beamRapidity[1]) > -0.2*/)
            {
              double phaseSpaceFactor = 2;
              hist_ptFull -> Fill(package.PtxRap(i)/(0.5*package.BeamRapidity()[1]), package.Weight(i)*phaseSpaceFactor);
              if(fabs(y0 - 0.25) < 0.25) 
                hist_pt -> Fill(package.PtxRap(i)/(0.5*package.BeamRapidity()[1]), package.Weight(i)*phaseSpaceFactor);
            }
          }
      });
    ana_hists[pdg] = hist_ana;
  }

  this -> RegisterFinishTaskRule([ana_hists, this]()
    {
      std::map<int, TH1D*> ptHists;
      for(int i = 0; i < fSupportedPDG.size(); ++i)
      {
        const auto hist = ana_hists.at(fSupportedPDG[i]);
        auto HistName = TString::Format("%s", hist -> GetName());
        auto projx = hist -> ProjectionX(TString::Format("%s_Rap", HistName.Data()));
        auto projy = hist -> ProjectionY(TString::Format("%s_Pt", HistName.Data()),
                                         hist -> GetXaxis() -> FindBin(double(0.5)), 
                                         hist -> GetXaxis() -> FindBin(double(1)));
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

      CIN -> Write();
      CIP -> Write();
      auto CINCIP = (TH1D*) CIN -> Clone("CINCIP_ana_Pt");
      CINCIP -> Divide(CIP);
      CINCIP -> Write();

      // save the drawing parameters
      TParameter<double>("ProbThresholdForRap", fProbThresholdForRap).Write();
      TParameter<double>("SDThresholdForRap", fSDThresholdForRap).Write();
      TParameter<double>("EffThresholdForRap", fEffThresholdForRap).Write();
      TParameter<double>("PtThresholdForRap", fPtThresholdForRap).Write();
    });
}

void STSimpleGraphsTask::RegisterPionPlots()
{
  fPlotPion = true;
  double kebins[] = {0, 30, 60, 90, 120, 170, 240};
  auto pim = this -> RegisterHistogram<TH1F>(false, "pimkecm", "Pi- CM K.E.", 6, kebins);
  pim -> Sumw2();
  auto pim_err = this -> RegisterHistogram<TH1F>(false, "pimkecm_err", "", 6, kebins);
  pim_err -> Sumw2();
  auto pip = this -> RegisterHistogram<TH1F>(false, "pipkecm", "Pi+ CM K.E.", 6, kebins);
  pip -> Sumw2();
  auto pip_err = this -> RegisterHistogram<TH1F>(false, "pipkecm_err", "", 6, kebins);
  pip_err -> Sumw2();

  auto pion_mass = TDatabasePDG::Instance() -> GetParticle(211) -> Mass()*1000;

  this -> RegisterRuleWithParticle(211, [pip, pip_err, pion_mass](const DataPackage& package)
    {
      const auto& data = package.Data();
      for(int i = 0; i < data.multiplicity; ++i)
        if(package.Eff(i) > 0.05 && package.Prob(i) > 0.2)
        {
          //double y0 = package.fragRapidity(i)/(0.5*package.beamRapidity[1]);
          if(package.FragRapidity(i) > 0)
          {
            const double phaseSpaceFactor = 2; // assume that backward angle contribute the other half
            double eff = package.Eff(i)/phaseSpaceFactor;
            double prob = package.Prob(i);
            auto p = package.CMVector(i).Mag();
            auto ke = sqrt(p*p + pion_mass*pion_mass) - pion_mass;
            pip -> Fill(ke, prob/eff);
            pip_err -> Fill(ke, std::pow(prob/eff*package.EffErr(i)/eff, 2));
          }
       }
    });

  this -> RegisterRuleWithParticle(-211, [pim, pim_err, pion_mass](const DataPackage& package)
    {
      const auto& data = package.Data();
      for(int i = 0; i < data.multiplicity; ++i)
        if(package.Eff(i) > 0.05 && package.Prob(i) > 0.2)
        {
          //double y0 = package.fragRapidity(i)/(0.5*package.beamRapidity[1]);
          if(package.FragRapidity(i) > 0)//std::fabs(y0 - 0.25) < 0.25)
          {
            const double phaseSpaceFactor = 2; // assume that backward angle contribute the other half
            double eff = package.Eff(i)/phaseSpaceFactor;
            double prob = package.Prob(i);
            auto p = package.CMVector(i).Mag();
            auto ke = sqrt(p*p + pion_mass*pion_mass) - pion_mass;
            pim -> Fill(ke, prob/eff);
            pim_err -> Fill(ke, std::pow(prob/eff*package.EffErr(i)/eff, 2));
          }
       }
    });
 
  this -> RegisterFinishTaskRule([pip, pip_err, pim, pim_err, this]()
    {
      // error bar assignment
      for(int i = 1; i <= pim -> GetNbinsX(); ++i)
      {
        double effErrSquared = pim_err -> GetBinContent(i);
        double statErrSquared = std::pow(pim -> GetBinError(i), 2);
        double totalError = std::sqrt(effErrSquared + statErrSquared);
        pim -> SetBinError(i, totalError);
      }

      for(int i = 1; i <= pip -> GetNbinsX(); ++i)
      {
        double effErrSquared = pip_err -> GetBinContent(i);
        double statErrSquared = std::pow(pip -> GetBinError(i), 2);
        double totalError = std::sqrt(effErrSquared + statErrSquared);
        pip -> SetBinError(i, totalError);
      }

      auto pimpip = (TH1F*) f1DHists["pimkecm"] -> Clone("pimpip");
      pimpip -> Divide(f1DHists["pipkecm"]);
      pimpip -> Write();
    });
}

void STSimpleGraphsTask::RegisterVPlots()
{
  // this only works for proton
  for(int pdg : fSupportedPDG)
  {
    int nbins = (std::abs(pdg) == 211)? 13 : 25;
    auto v1_counts = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_rap_counts", fParticleName.at(pdg).c_str()), "", nbins, -2, 2);
    auto v2_counts = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_rap_counts", fParticleName.at(pdg).c_str()), "", nbins, -2, 2);
    auto v1 = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_cumsum", fParticleName.at(pdg).c_str()), "", nbins, -2, 2); 
    auto v2 = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_cumsum", fParticleName.at(pdg).c_str()), "", nbins, -2, 2); 
    auto v2_sechar = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_sechar_cumsum", fParticleName.at(pdg).c_str()), "", nbins, -2, 2);

    auto particle = TDatabasePDG::Instance() -> GetParticle(pdg);
    double mass = particle -> Mass()/STAnaParticleDB::kAu2Gev;

    this -> RegisterRuleWithParticle(pdg, 
      [this, v1, v2, v2_sechar, v1_counts, v2_counts,  mass](const DataPackage& package)
      {
        const auto& data = package.Data();
        for(int i = 0; i < data.multiplicity; ++i)
          if(package.Prob(i) > fProbThresholdForVs && package.PhiEff(i) > fPhiEffThresholdForVs)
          {
            if(package.CMVector(i).Perp()/package.BeamMom()[0]/mass > fPtThresholdForVs && !std::isnan(package.V1RPAngle(i)))
            {
              double y0 = package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);
              double phi = package.CMVector(i).Phi();
              double weight = package.Prob(i)/package.PhiEff(i);
              v1 -> Fill(y0, cos(phi - package.V1RPAngle(i))*weight);
              v1_counts -> Fill(y0);

              //phi = (phi < 0)? 2*TMath::Pi() + phi : phi; 
              v2 -> Fill(y0, cos(2*(package.V1RPAngle(i) - phi))*weight);
              v2_sechar -> Fill(y0, cos(2*(package.V2RPAngle(i) - phi))*weight);
              v2_counts -> Fill(y0);
            }

          }
      });

    this -> RegisterFinishTaskRule([v1, v1_counts, v2, v2_sechar, v2_counts, pdg, this]()
      {
        auto v1_cloned = (TH1F*) v1 -> Clone(TString::Format("%s_v1", fParticleName.at(pdg).c_str()));
        v1_cloned -> Divide(v1_counts);
        v1_cloned -> Write();

        auto v2_cloned = (TH1F*) v2 -> Clone(TString::Format("%s_v2", fParticleName.at(pdg).c_str()));
        v2_cloned -> Divide(v2_counts);
        v2_cloned -> Write();

        v2_cloned = (TH1F*) v2_sechar -> Clone(TString::Format("%s_v2_sechar", fParticleName.at(pdg).c_str()));
        v2_cloned -> Divide(v2_counts);
        v2_cloned -> Write();

        TParameter<double>("PtThresholdForVs", fPtThresholdForVs).Write();
        TParameter<double>("ProbThresholdForVs", fProbThresholdForVs).Write();
        TParameter<double>("PhiEffThresholdForVs", fPhiEffThresholdForVs).Write();
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
        pdg_hists[i][j] = this -> RegisterHistogram<TH2F>(true, TString::Format("Pitch%dYaw%dPDG%d", i, j, pdg), "", 
                                                          fMomBins, fMinMom, fMaxMom, 
                                                          fdEdXBins, fMindEdX, fMaxdEdX);

    this -> RegisterRuleWithParticle(pdg,
      [pdg_hists, this](const DataPackage& package)
      {
        const auto& data = package.Data();
        for(int i = 0; i < data.multiplicity; ++i)
        {
          int pitchID = this -> _ToPitchId(data.vaMom[i]);
          int yawId = this -> _ToYawId(data.vaMom[i]);
          if(data.vaNRowClusters[i] + data.vaNLayerClusters[i] > fMinNClus && 
             data.recodpoca[i].Mag() < fMaxDPOCA)
            pdg_hists[pitchID][yawId] -> Fill(data.vaMom[i].Mag(), 
                                              data.vadedx[i], package.Prob(i));
        }
      });
  }

  this -> RegisterFinishTaskRule([this]()
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
    });  


}

void STSimpleGraphsTask::RegisterPlotsForMC()
{
  this -> RegisterRuleForEachParticle<TH1F>([](const DataPackage& package, TH1* hist, int pdg)
    {
      double multiplicity = 0;
      const auto& data = package.Data();
      for(int i = 0; i < data.multiplicity; ++i)
        if(package.Prob(i) > 0.2 && package.Eff(i) > 0.05 && package.FragRapidity(i) > 0)
          multiplicity += package.Weight(i)*2; // 2 is the phase space factor for package.fragRapidity(i) > 0
      hist -> Fill(multiplicity);
    }, "_mult", "", 11,0,95);
  
  this -> RegisterRuleForEachParticle<TH2F>([](const DataPackage& package, TH1* hist, int pdg)
    {
      const auto& data = package.Data();
      for(int i = 0; i < data.multiplicity; ++i)
        if(package.Prob(i) > 0.2 && package.Eff(i) > 0.05 && package.FragRapidity(i) > 0)
          static_cast<TH2F*>(hist) -> Fill(package.FragVelocity(i).z(), package.FragVelocity(i).Perp(), package.Weight(i));
    }, "_CMVel", "", 50,0,1,50,0,1);
}

struct ParticleInfo{ int A; int Z; double px; double py; double pz; };

void STSimpleGraphsTask::CreateMCEventsFromHist(const std::string& forwardFile, const std::string& backwardFile,
                                                const std::string& outputFile, int nevent, double energyPerN,
                                                int multMin, int multMax, const std::string& simPara)
{
  // type of particles to simulate
  const auto& supportedPDG = STAnaParticleDB::GetSupportedPDG();
  STAnaParticleDB::FillTDatabasePDG();

  // get beam NN rapidity
  double totalE = energyPerN + STAnaParticleDB::kAu2Gev;
  double pzBeam = sqrt(totalE*totalE - STAnaParticleDB::kAu2Gev*STAnaParticleDB::kAu2Gev);
  TLorentzVector beamVec(0, 0, pzBeam, totalE);
  double beamRapidity = 0.5*beamVec.Rapidity();
  auto beamVel = beamVec.BoostVector();

  auto MergeHist = [](TH2F* front, TH2F* back)
  {
    // flip back histogram around x = 0, then glue iwth with front along x = 0 to create one single histogram
    TH2F *merged_hist = new TH2F(TString::Format("%s_merged", front -> GetName()), back -> GetTitle(), 
                                                 2*back -> GetNbinsX(), 
                                                 -back -> GetXaxis() -> GetXmax(), 
                                                 front -> GetXaxis() -> GetXmax(), 
                                                 2*back -> GetNbinsY(), 
                                                 front -> GetYaxis() -> GetXmin(), 
                                                 front -> GetYaxis() -> GetXmax());
  
    for(int i = 1; i < merged_hist -> GetNbinsX(); ++i)
      for(int j = 1; j < merged_hist -> GetNbinsY(); ++j)
      {
        double x = merged_hist -> GetXaxis() -> GetBinCenter(i);
        double y = merged_hist -> GetYaxis() -> GetBinCenter(j);
        if(x > 0) merged_hist -> SetBinContent(i, j, front -> Interpolate(x, y));
        else merged_hist -> SetBinContent(i, j, back -> Interpolate(-x, y));
      }
    return merged_hist;
  };

  // functional form for v1 and v2
  TF1 v1("v1", "[0]/(1+exp(x*[1])) + [2]"), v2("v2", "[0] + [1]*x*x");
  const int v1Npar = v1.GetNpar();
  const int v2Npar = v2.GetNpar();
  // equation that needed solving to add v1 and v2
  // for details, read https://journals.aps.org/prc/pdf/10.1103/PhysRevC.79.064909
  TF1 angEq("angEq", "x + 2*[0]*sin(x) + [1]*sin(2*x)", -TMath::Pi(), TMath::Pi());

  std::vector<std::vector<double>> v1_params(supportedPDG.size(), std::vector<double>(v1Npar, 0)), 
                                   v2_params(supportedPDG.size(), std::vector<double>(v2Npar, 0));
  double pimpip_ratio = 0;
  int pion_rebin = 1; // pion has lower statistics. Need to rebin for smooth phase space
  // load parameters from simPara to add additional information for simulation
  {
    TDOMParser parser;
    parser.SetValidate(false);
    parser.ParseFile(simPara.c_str());
    auto node = parser.GetXMLDocument()->GetRootNode()->GetChildren();
    for(; node; node = node->GetNextNode())
      if(node -> GetNodeType() == TXMLNode::kXMLElementNode)
      {
        if(std::strcmp(node -> GetNodeName(), "ParticleType") == 0)
        {
          // load info into map for easy access
          std::map<std::string, std::string> PartInfo; 
          for(auto vNode = node -> GetChildren(); vNode; vNode = vNode -> GetNextNode()) 
            if(vNode -> GetNodeType() == TXMLNode::kXMLElementNode) 
              PartInfo[std::string(vNode->GetNodeName())] = vNode->GetText();
            // find its location in supportedPDG
          auto it = std::find(supportedPDG.begin(), supportedPDG.end(), std::stoi(PartInfo.at("PID")));
          if(it != supportedPDG.end())
          {
            int partOrder = std::distance(supportedPDG.begin(), it);
            auto& v1_para = v1_params[partOrder];
            auto& v2_para = v2_params[partOrder];
            for(int paraType = 0; paraType < v1Npar; ++paraType)  
              v1_para[paraType] = std::stof(PartInfo.at("v1_" + std::to_string(paraType)));
            for(int paraType = 0; paraType < v2Npar; ++paraType)  
              v2_para[paraType] = std::stof(PartInfo.at("v2_" + std::to_string(paraType)));
          }
        }       
        else if(std::strcmp(node -> GetNodeName(), "PimPipRatio") == 0)
        {  pimpip_ratio = std::atof(node -> GetText()); }
        else if(std::strcmp(node -> GetNodeName(), "PionRebin") == 0)
        { pion_rebin = std::atoi(node -> GetText()); }

      }
  }

  // load particle distributions
  TFile distForwardFile(forwardFile.c_str());
  TFile distBackwardFile(backwardFile.c_str());
  std::vector<TH2F*> hists;
  std::vector<TH1F*> mults_forward, mults_backward;
  for(int pdg : supportedPDG)
  {
    auto pname = fParticleName.at(pdg);
    hists.push_back(MergeHist((TH2F*) distForwardFile.Get((pname + "_CMVel").c_str()),
                              (TH2F*) distBackwardFile.Get((pname + "_CMVel").c_str())));
    if(fabs(pdg) == 211) hists.back() -> Rebin2D(pion_rebin);

    mults_forward.push_back((TH1F*) distForwardFile.Get((pname + "_mult").c_str()));
    mults_backward.push_back((TH1F*) distBackwardFile.Get((pname + "_mult").c_str()));
  }

   // files output
  TFile output(outputFile.c_str(), "RECREATE");
  TTree outtree("cluster", "cluster");
  double px, py, pz, x = 0, y = 0, z = 0; 
  int A, Z, eventid;

  outtree.Branch("eventid", &eventid);
  outtree.Branch("x", &x);
  outtree.Branch("y", &y);
  outtree.Branch("z", &z);
  outtree.Branch("px", &px);
  outtree.Branch("py", &py);
  outtree.Branch("pz", &pz);
  outtree.Branch("A", &A);
  outtree.Branch("Z", &Z);
  
  for(eventid = 0; eventid < nevent; ++eventid)
  {
    std::vector<ParticleInfo> pList;
    int sum = 0; // count number of particles with pzLab > 0
    while(sum < multMin*1.3 || sum > multMax*1.3)
    {
      sum = 0;
      pList.clear();
      for(int ipart = 0; ipart < supportedPDG.size(); ++ipart)
      {
        // first determine how many particles are created
        int mult = int(0.5*(mults_forward[ipart] -> GetRandom() + mults_backward[ipart] -> GetRandom())); // multiply by two assuming pzCM is symmetric around zero
        if(hists[ipart] -> Integral() == 0) mult = 0;
        // pion needs to be treated specially. Make sure there are at lease one pim per events for statistics purposes
        if(supportedPDG[ipart] == -211)  mult = 1;
        else if(supportedPDG[ipart] == 211) mult = (gRandom -> Uniform() > 1/pimpip_ratio)? 0 : 1; // it is assumed that pim/pip ratio > 1
        for(int paraType = 0; paraType < v1Npar; ++paraType)  
          v1.SetParameter(paraType, v1_params[ipart][paraType]);
        for(int paraType = 0; paraType < v2Npar; ++paraType)  
          v2.SetParameter(paraType, v2_params[ipart][paraType]);
        auto particle = TDatabasePDG::Instance()->GetParticle(supportedPDG[ipart]);

        // determine CM momentum
        for(int imult = 0; imult < mult; ++imult)
        {
          double vz = 1, vpt = 1;
          while(sqrt(vz*vz + vpt*vpt) > 1) hists[ipart] -> GetRandom2(vz, vpt);
          //if(gRandom -> Uniform(0,1) > 0.5) vz *= -1; // again, symmetric pzCM along zero is assumed
          //double v2 = vz*vz + vpt*vpt;
          double gamma = 1/sqrt(1 - vz*vz - vpt*vpt);
          double mass = particle -> Mass();

          TVector3 momCM(gamma*mass*vpt, 0, gamma*mass*vz); // MeV/c to GeV/c

          // determine the azimuthal angle. Assume uniform distribution. All information on reaction plane is ignored.
          TLorentzVector partLV;
          partLV.SetVectM(momCM, mass);
          double v1_par = v1.Eval(partLV.Rapidity()/beamRapidity);
          double v2_par = v2.Eval(partLV.Rapidity()/beamRapidity);
          angEq.SetParameters(v1_par, v2_par);
          double delta_phi = gRandom -> Uniform(-TMath::Pi(), TMath::Pi());
          // add flow to uniform phi distribution
          delta_phi = angEq.GetX(delta_phi, -TMath::Pi(), TMath::Pi(), 1e-2);
          //delta_phi = delta_phi - 2*v1_par*sin(delta_phi) - v2_par*sin(2*delta_phi);
          momCM.RotateZ(delta_phi);
          ParticleInfo info;

          info.px = momCM.x();
          info.py = momCM.y();
          info.pz = momCM.z();
          info.A = std::round(particle -> Mass()/STAnaParticleDB::kAu2Gev);
          info.Z = std::round(particle -> Charge()/3);

          pList.push_back(info);

          partLV.Boost(beamVel);
          if(partLV.Z() > 0) ++sum;
        }
      }

      // count number of particles with pzLab > 0
      //for(const auto& particle : pList) ++sum;
      //  if(particle.pz > -0.360*particle.A) ++sum;
    }

    for(const auto& particle : pList)
    {
      px = particle.px;
      py = particle.py;
      pz = particle.pz;
      A = particle.A;
      Z = particle.Z;
      outtree.Fill();
    }
    std::cout << "Processing event " << eventid << "\r" << std::flush;
  }
  std::cout << "\n";
  output.cd();
  outtree.Write();
}
