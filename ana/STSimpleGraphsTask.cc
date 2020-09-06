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

void STSimpleGraphsTask::RegisterRapidityPlots(double probThreshold, double SDThreshold)
{
  fPlotRapidity = true;
  STAnaParticleDB::FillTDatabasePDG();
  std::map<int, TH2F*> ana_hists;
  for(auto& pdg : fSupportedPDG)
  {
    auto pname = fParticleName[pdg];
    auto hist_ana = this -> RegisterHistogram<TH2F>(true, TString::Format("%s_ana", pname.c_str()),
                                                     ";2y_{z}/y_{beam Lab};CM P_{t}/A (MeV/c2);",
                                                     200, -4, 4, 60, 0, 1000);
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
      [this, hist_ana, hist_rap, hist_pt, hist_ptFull, pmass, pdg, probThreshold, SDThreshold](const DataPackage& package)
      {
        const auto& data = package.Data();
        for(int i = 0; i < data.multiplicity; ++i)
          if(package.Eff(i) > 0.05 && package.Prob(i) > probThreshold && std::fabs(package.StdDev(i)) < SDThreshold)
          {
            double y0 = package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);
            auto minMom = fMinMomForCMInLab[pdg];
            if(data.vaMom[i].Mag() > minMom)
            {
              hist_ana -> Fill(y0, package.CMVector(i).Perp()/pmass, package.Weight(i));
              hist_rap -> Fill(y0, package.Weight(i));
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
                                         hist -> GetXaxis() -> FindBin(double(0)), 
                                         hist -> GetXaxis() -> FindBin(double(0.5)));
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
    auto v1_counts = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_rap_counts", fParticleName[pdg].c_str()), "", 25, -2, 2);
    auto v2_counts = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_rap_counts", fParticleName[pdg].c_str()), "", 25, -2, 2);
    auto v1 = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_cumsum", fParticleName[pdg].c_str()), "", 25, -2, 2); 
    auto v2 = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_cumsum", fParticleName[pdg].c_str()), "", 25, -2, 2); 

    auto particle = TDatabasePDG::Instance() -> GetParticle(pdg);
    double mass = particle -> Mass()/STAnaParticleDB::kAu2Gev;

    this -> RegisterRuleWithParticle(pdg, 
      [v1, v2, v1_counts, v2_counts, mass](const DataPackage& package)
      {
        const auto& data = package.Data();
        for(int i = 0; i < data.multiplicity; ++i)
          if(package.Prob(i) > 0.95 && package.PhiEff(i) > 0)
          {
            if(package.CMVector(i).Perp()/package.BeamMom()[0]/mass > 0.4)
            {
              double y0 = package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);
              double phi = package.CMVector(i).Phi();
              double weight = package.Prob(i)/package.PhiEff(i);
              v1 -> Fill(y0, cos(phi - package.V1RPAngle(i))*weight);
              v1_counts -> Fill(y0);

              //phi = (phi < 0)? 2*TMath::Pi() + phi : phi; 
              double v2_phi = package.V1RPAngle(i);
              double diff = phi - v2_phi;
              //if(diff < 0) diff += 2*TMath::Pi();
              //diff = std::fmod(diff, TMath::Pi()/2);
              v2 -> Fill(y0, cos(2*diff)*weight);
              v2_counts -> Fill(y0);
            }

          }
      });

    this -> RegisterFinishTaskRule([v1, v1_counts, v2, v2_counts, pdg, this]()
      {
        auto v1_cloned = (TH1F*) v1 -> Clone(TString::Format("%s_v1", fParticleName[pdg].c_str()));
        v1_cloned -> Divide(v1_counts);
        v1_cloned -> Write();

        auto v2_cloned = (TH1F*) v2 -> Clone(TString::Format("%s_v2", fParticleName[pdg].c_str()));
        v2_cloned -> Divide(v2_counts);
        v2_cloned -> Write();
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
