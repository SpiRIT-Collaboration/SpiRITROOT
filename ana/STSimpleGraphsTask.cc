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
#include <unordered_map>

const std::map<int, std::string> STSimpleGraphsTask::fParticleName{{2212, "p"}, 
                                                                   {1000010020, "d"}, 
                                                                   {1000010030, "t"}, 
                                                                   {1000020030, "He3"}, 
                                                                   {1000020040, "He4"}, 
                                                                   {1000020060, "He6"},
                                                                   {211, "pip"},
                                                                   {-211, "pim"},
                                                                   {2112, "n"}};



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
        eff -> fElements.resize(500, def_val.second);
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

  if(fNNFrame) fLogger -> Info(MESSAGE_ORIGIN, "Will use NN-frame rapidity if applicable.");

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

void STSimpleGraphsTask::RegisterMCNPPlots()
{
  STAnaParticleDB::FillTDatabasePDG();
  if(std::find(fSupportedPDG.begin(), fSupportedPDG.end(), 2112) == fSupportedPDG.end())
  {
    std::cerr << "Neutrons are not added to the list of supported PDG. Cannot plot MC NP ratio.\n";
    return;
  }

  auto CIN = this -> RegisterHistogram<TH1F>(true, TString::Format("CIN_MC"), 
                                             ";P_{t}/A;", 12, 0, 1000);
  auto CIP = this -> RegisterHistogram<TH1F>(true, TString::Format("CIP_MC"), 
                                             ";P_{t}/A;", 12, 0, 1000);
  auto CIN_rap = this -> RegisterHistogram<TH1F>(true, TString::Format("CIN_MC_Rap"), 
                                                 ";P_{t}/A;", 200, -4, 4);
  auto CIP_rap = this -> RegisterHistogram<TH1F>(true, TString::Format("CIP_MC_Rap"), 
                                                 ";P_{t}/A;", 200, -4, 4);



  for(auto& pdg : fSupportedPDG)
  {
    auto particle = TDatabasePDG::Instance() -> GetParticle(pdg);
    auto pmass = int(particle -> Mass()/STAnaParticleDB::kAu2Gev + 0.5);
    int charge = int(particle -> Charge()/3 + 0.5);
    pmass = (pmass == 0)? 1 : std::abs(pmass);
 
    this -> RegisterRuleWithParticle(pdg, [this, CIN, CIP, CIN_rap, CIP_rap, pmass, charge](const DataPackage& package)
      {
        const auto& data = package.Data();
        for(int i = 0; i < data.multiplicity; ++i)
          if(package.Eff(i) > 0.5 && package.Prob(i) > 0.5 && package.CMVector(i).Perp() > fPtThresholdForMC)
          {
            double y0 = (fNNFrame)? package.LabRapidity(i)/(0.5*package.BeamRapidity()[1]) - 1 : package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);//package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);
            if(std::fabs(y0) < 0.5)
            {
              CIN -> Fill(package.CMVector(i).Perp()/pmass, (pmass - charge)*package.Weight(i));
              CIP -> Fill(package.CMVector(i).Perp()/pmass, charge*package.Weight(i));
            }
            CIN_rap -> Fill(y0, (pmass - charge)*package.Weight(i));
            CIP_rap -> Fill(y0, charge*package.Weight(i));
          }
      });
  }

  this -> RegisterFinishTaskRule([this, CIN, CIP]()
    {
      auto CINCIP = (TH1F*) CIN -> Clone("CINCIP_MC");
      CINCIP -> Divide(CIP);
      CINCIP -> Write();

      TParameter<double>("PtThresholdForMC", fPtThresholdForMC).Write();
    });

}

void STSimpleGraphsTask::RegisterRapidityPlots()
{
  fPlotRapidity = true;
  STAnaParticleDB::FillTDatabasePDG();
  std::map<int, TH2F*> ana_hists;
  std::map<int, TH1F*> ptFullHists;
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
    auto hist_keTheta = this -> RegisterHistogram<TH2F>(true, TString::Format("%s_KeTheta", pname.c_str()),
                                                        ";E_{CM}/A (MeV/A);#theta (deg);", 
							200, 0, 400, 200, 0, 180);
    auto particle = TDatabasePDG::Instance() -> GetParticle(pdg);
    auto pmass = int(particle -> Mass()/STAnaParticleDB::kAu2Gev);
    pmass = (pmass == 0)? 1 : std::abs(pmass);

    this -> RegisterRuleWithParticle(pdg, 
      [this, hist_ana, hist_rap, hist_pt, hist_ptFull, hist_keTheta, pmass, pdg](const DataPackage& package)
      {
        const auto& data = package.Data();
        for(int i = 0; i < data.multiplicity; ++i)
          if(package.Eff(i) > fEffThresholdForRap && 
             package.Prob(i) > ((pdg == 1000020030)? fProbHe3ThresholdForRap : fProbThresholdForRap) && 
             std::fabs(package.StdDev(i)) < fSDThresholdForRap)
          {
            double y0 = (fNNFrame)? package.LabRapidity(i)/(0.5*package.BeamRapidity()[1]) - 1 : package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);
            auto minMom = fMinMomForCMInLab[pdg];
            if(data.vaMom[i].Mag() > minMom)
            {
              hist_ana -> Fill(y0, package.CMVector(i).Perp()/pmass, package.Weight(i));
	      double realMass = pmass*STAnaParticleDB::kAu2Gev*1000; // in MeV
	      hist_keTheta -> Fill((std::sqrt(package.CMVector(i).Mag2() + realMass*realMass) - realMass)/pmass,
	                           package.CMVector(i).Theta()*TMath::RadToDeg(), package.Weight(i));
              if(package.CMVector(i).Perp() > fPtThresholdForRap) hist_rap -> Fill(y0, package.Weight(i));
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
    ptFullHists[pdg] = hist_ptFull;
  }

  this -> RegisterFinishTaskRule([ana_hists, ptFullHists, this]()
    {
      std::map<int, TH1D*> ptHists;
      TH1F *CIP_rap = nullptr;
      TH1F *CIP_pt = nullptr;
      TH1F *p_rap = nullptr;
      TH1F *d_rap = nullptr;
      TH1F *t_rap = nullptr;

      for(int i = 0; i < fSupportedPDG.size(); ++i)
      {
        const auto hist = ana_hists.at(fSupportedPDG[i]);
        auto HistName = TString::Format("%s", hist -> GetName());
        auto projx = hist -> ProjectionX(TString::Format("%s_Rap", HistName.Data()));
        auto projy = hist -> ProjectionY(TString::Format("%s_Pt", HistName.Data()),
                                         hist -> GetXaxis() -> FindBin(double(0.)), 
                                         hist -> GetXaxis() -> FindBin(double(0.5)));
        auto projy_side = hist -> ProjectionY(TString::Format("%s_Pt_0.5_y_1", HistName.Data()),
                                              hist -> GetXaxis() -> FindBin(double(0.5)), 
                                              hist -> GetXaxis() -> FindBin(double(1)));

        double dx = hist -> GetXaxis() -> GetBinWidth(0);
        double dy = hist -> GetYaxis() -> GetBinWidth(0);
        //projy -> Rebin();
        //projy -> Scale(0.5*dx);
        projy -> Write();

        projy_side -> Rebin();
        projy_side -> Scale(0.5*dx);
        projy_side -> Write();

        projx -> Rebin();
        projx -> Scale(0.5*dy);
        projx -> Write();
        ptHists[fSupportedPDG[i]] = projy;

        auto temp = (TH1F*) ptFullHists.at(fSupportedPDG[i]) -> Clone("temp");
        temp -> Rebin();
        temp -> Scale(0.5);

        if(fSupportedPDG[i] == 2212) p_rap = (TH1F*) projx -> Clone("p_rapHistClone");
        if(fSupportedPDG[i] == 1000010020) d_rap = (TH1F*) projx -> Clone("dp_rapHist");
        if(fSupportedPDG[i] == 1000010030) t_rap = (TH1F*) projx -> Clone("tp_rapHist");

        if(!CIP_rap && !CIP_pt) 
        {
          CIP_rap = (TH1F*) projx -> Clone("CIP_rapHist");
          CIP_pt = (TH1F*) temp -> Clone("CIP_ptFullHist");
        }
        else if(fSupportedPDG[i] == 2212 || fSupportedPDG[i] == 1000010020 || fSupportedPDG[i] == 1000010030)
        {
          CIP_rap -> Add(projx);
          CIP_pt -> Add(temp);
        }
        else if(fSupportedPDG[i] == 1000020030 || fSupportedPDG[i] == 1000020040)
        {
          projx -> Scale(2);
          CIP_rap -> Add(projx);

          temp -> Scale(2);
          CIP_pt -> Add(temp);
        }
        temp -> Delete();
      }

      if(d_rap && p_rap) 
      {
        d_rap -> Divide(p_rap);
        d_rap -> Write();
      }
      if(t_rap && p_rap)
      {
        t_rap -> Divide(p_rap);
        t_rap -> Write();
      }

      if(CIP_rap && CIP_pt) 
      {
        CIP_rap -> Write();
        CIP_pt -> Write();
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
      TParameter<double>("ProbHe3ThresholdForRap", fProbHe3ThresholdForRap).Write();
      TParameter<double>("SDThresholdForRap", fSDThresholdForRap).Write();
      TParameter<double>("EffThresholdForRap", fEffThresholdForRap).Write();
      TParameter<double>("PtThresholdForRap", fPtThresholdForRap).Write();
    });
}

void STSimpleGraphsTask::RegisterPionPlots()
{
  fPlotPion = true;
  double kebins[] = {0, 15, 30, 45, 60, 75, 90, 105, 120, 145, 170, 205, 240};
  auto pim = this -> RegisterHistogram<TH1F>(false, "pimkecm", "Pi- CM K.E.", 12, kebins);
  pim -> Sumw2();
  auto pim_err = this -> RegisterHistogram<TH1F>(false, "pimkecm_err", "", 12, kebins);
  pim_err -> Sumw2();
  auto pip = this -> RegisterHistogram<TH1F>(false, "pipkecm", "Pi+ CM K.E.", 12, kebins);
  pip -> Sumw2();
  auto pip_err = this -> RegisterHistogram<TH1F>(false, "pipkecm_err", "", 12, kebins);
  pip_err -> Sumw2();

  double ptbins[] = {0, 20, 40, 60, 80, 100, 120, 140, 160, 180, 200, 220, 240, 280, 320, 440};
  auto pim_pt = this -> RegisterHistogram<TH1F>(false, "pimptcm", "Pi- CM P_{t}", 15, ptbins);
  pim_pt -> Sumw2();
  auto pim_pt_err = this -> RegisterHistogram<TH1F>(false, "pimptcm_err", "", 15, ptbins);
  pim_pt_err -> Sumw2();
  auto pip_pt = this -> RegisterHistogram<TH1F>(false, "pipptcm", "Pi- CM P_{t}", 15, ptbins);
  pip_pt -> Sumw2();
  auto pip_pt_err = this -> RegisterHistogram<TH1F>(false, "pipptcm_err", "", 15, ptbins);
  pip_pt_err -> Sumw2();

  auto pim_RP = this -> RegisterHistogram<TH1F>(true, "pim_RPCut", ";2y_{z}/y_{beam Lab};", 200, -4, 4);
  pim_RP -> Sumw2();
  auto pip_RP = this -> RegisterHistogram<TH1F>(true, "pip_RPCut", ";2y_{z}/y_{beam Lab};", 200, -4, 4);
  pip_RP -> Sumw2();


  auto pion_mass = TDatabasePDG::Instance() -> GetParticle(211) -> Mass()*1000;

  this -> RegisterRuleWithParticle(211, [this, pip, pip_err, pip_pt, pip_pt_err, pion_mass, pip_RP](const DataPackage& package)
    {
      const auto& data = package.Data();
      for(int i = 0; i < data.multiplicity; ++i)
        if(package.Eff(i) > 0.05 && package.Prob(i) > 0.2)
        {
          //double y0 = package.fragRapidity(i)/(0.5*package.beamRapidity[1]);
          if(package.FragRapidity(i) > 0)
          {
            double y0 = (fNNFrame)? package.LabRapidity(i)/(0.5*package.BeamRapidity()[1]) - 1 : package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);
            const double phaseSpaceFactor = 2; // assume that backward angle contribute the other half
            double eff = package.Eff(i)/phaseSpaceFactor;
            double prob = package.Prob(i);
            auto p = package.CMVector(i).Mag();
            auto ke = sqrt(p*p + pion_mass*pion_mass) - pion_mass;
            pip -> Fill(ke, prob/eff);
            pip_err -> Fill(ke, std::pow(prob/eff*package.EffErr(i)/eff, 2));

            auto pt = package.CMVector(i).Perp();
            pip_pt -> Fill(pt, prob/eff);
            pip_pt_err -> Fill(pt, std::pow(prob/eff*package.EffErr(i)/eff, 2));

            if(fabs(package.CMVector(i).Phi()) < fPhiRangeForPi || fabs(package.CMVector(i).Phi()) > M_PI - fPhiRangeForPi) {
              double moddedRP = std::fmod(package.V1RPAngle(i) + M_PI, M_PI);
              if(std::fabs(moddedRP - fRPAngleForPi) < fRPRangeForPi || std::fabs(moddedRP - fRPAngleForPi) > M_PI - fRPRangeForPi) {
                pip_RP -> Fill(y0, /*package.CMVector(i).Perp(),*/ package.Weight(i));
              }
            }
          }
       }
    });

  this -> RegisterRuleWithParticle(-211, [this, pim, pim_err, pim_pt, pim_pt_err, pion_mass, pim_RP](const DataPackage& package)
    {
      const auto& data = package.Data();
      for(int i = 0; i < data.multiplicity; ++i)
        if(package.Eff(i) > 0.05 && package.Prob(i) > 0.2)
        {
          //double y0 = package.fragRapidity(i)/(0.5*package.beamRapidity[1]);
          if(package.FragRapidity(i) > 0)//std::fabs(y0 - 0.25) < 0.25)
          {
            double y0 = (fNNFrame)? package.LabRapidity(i)/(0.5*package.BeamRapidity()[1]) - 1 : package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);
            const double phaseSpaceFactor = 2; // assume that backward angle contribute the other half
            double eff = package.Eff(i)/phaseSpaceFactor;
            double effErr = package.EffErr(i)/phaseSpaceFactor;
            double prob = package.Prob(i);
            auto p = package.CMVector(i).Mag();
            auto ke = sqrt(p*p + pion_mass*pion_mass) - pion_mass;
            pim -> Fill(ke, prob/eff);
            pim_err -> Fill(ke, std::pow(prob/eff*effErr/eff, 2));

            auto pt = package.CMVector(i).Perp();
            pim_pt -> Fill(pt, prob/eff);
            pim_pt_err -> Fill(pt, std::pow(prob/eff*effErr/eff, 2));

            if(fabs(package.CMVector(i).Phi()) < fPhiRangeForPi || fabs(package.CMVector(i).Phi()) > M_PI - fPhiRangeForPi) {
              double moddedRP = std::fmod(package.V1RPAngle(i) + M_PI, M_PI);
              if(std::fabs(moddedRP - fRPAngleForPi) < fRPRangeForPi || std::fabs(moddedRP - fRPAngleForPi) > M_PI - fRPRangeForPi) {
                pim_RP -> Fill(y0, /*package.CMVector(i).Perp(),*/ package.Weight(i));
              }
            }
          }
       }
    });
 
  this -> RegisterFinishTaskRule([pip, pip_err, pip_pt, pip_pt_err, pim, pim_err, pim_pt, pim_pt_err, this]()
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

      for(int i = 1; i <= pim_pt -> GetNbinsX(); ++i)
      {
        double effErrSquared = pim_pt_err -> GetBinContent(i);
        double statErrSquared = std::pow(pim_pt -> GetBinError(i), 2);
        double totalError = std::sqrt(effErrSquared + statErrSquared);
        pim_pt -> SetBinError(i, totalError);
      }

      for(int i = 1; i <= pip_pt -> GetNbinsX(); ++i)
      {
        double effErrSquared = pip_pt_err -> GetBinContent(i);
        double statErrSquared = std::pow(pip_pt -> GetBinError(i), 2);
        double totalError = std::sqrt(effErrSquared + statErrSquared);
        pip_pt -> SetBinError(i, totalError);
      }

      auto pimpip = (TH1F*) f1DHists["pimkecm"] -> Clone("pimpip");
      pimpip -> Divide(f1DHists["pipkecm"]);
      pimpip -> Write();

      pimpip = (TH1F*) f1DHists["pimptcm"] -> Clone("pimpip_pt");
      pimpip -> Divide(f1DHists["pipptcm"]);
      pimpip -> Write();

      TParameter<double>("fRPAngleForPi", fRPAngleForPi).Write();
      TParameter<double>("fRPRangeForPi", fRPRangeForPi).Write();
      TParameter<double>("fPhiRangeForPi", fPhiRangeForPi).Write();
    });
}

void STSimpleGraphsTask::RegisterVPlots()
{
  auto CIP_v1_counts = this -> RegisterHistogram<TH1F>(false, "CIP_v1_rap_counts", "", 25, -2, 2);
  auto CIP_v1 = this -> RegisterHistogram<TH1F>(false, "CIP_v1_cumsum", "", 25, -2, 2);
  auto CIP_pt_v1_counts = this -> RegisterHistogram<TH1F>(false, "CIP_v1_pt_counts", "", 10, 0, 1000);
  auto CIP_pt_v1 = this -> RegisterHistogram<TH1F>(false, "CIP_v1_pt_cumsum", "", 10, 0, 1000);
  auto CIP_v2_counts = this -> RegisterHistogram<TH1F>(false, "CIP_v2_rap_counts", "", 25, -2, 2);
  auto CIP_v2 = this -> RegisterHistogram<TH1F>(false, "CIP_v2_cumsum", "", 25, -2, 2);


  // this only works for proton
  for(int pdg : fSupportedPDG)
  {
    int nbins = 25;
    double yRange = (std::abs(pdg) == 211)? 4 : 2;
    auto v1_counts = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_rap_counts", fParticleName.at(pdg).c_str()), "", nbins, -yRange, yRange);
    auto v2_counts = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_rap_counts", fParticleName.at(pdg).c_str()), "", nbins, -yRange, yRange);
    auto v1 = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_cumsum", fParticleName.at(pdg).c_str()), "", nbins, -yRange, yRange); 
    auto v2 = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_cumsum", fParticleName.at(pdg).c_str()), "", nbins, -yRange, yRange); 

    int pt_nbins = 10;
    auto v1_pt_counts = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_pt_counts", fParticleName.at(pdg).c_str()), "", pt_nbins, 0, 1000);
    auto v1_pt = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_pt_cumsum", fParticleName.at(pdg).c_str()), "", pt_nbins, 0, 1000);

    TH1F *v1_withPtCut = nullptr;
    TH1F *v2_withPtCut = nullptr;
    TH1F *v1_counts_withPtCut = nullptr;
    TH1F *v2_counts_withPtCut = nullptr;
    TH1F *v1_withLEPtCut = nullptr;
    TH1F *v2_withLEPtCut = nullptr;
    TH1F *v1_counts_withLEPtCut = nullptr;
    TH1F *v2_counts_withLEPtCut = nullptr;



    if(fPtThresholdForVs > 0)
    {
      v1_withPtCut = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_Pt%g_cumsum", fParticleName.at(pdg).c_str(), fPtThresholdForVs), "", nbins, -yRange, yRange);
      v2_withPtCut = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_Pt%g_cumsum", fParticleName.at(pdg).c_str(), fPtThresholdForVs), "", nbins, -yRange, yRange);

      v1_counts_withPtCut = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_rap_counts_Pt%g", fParticleName.at(pdg).c_str(), fPtThresholdForVs), "", nbins, -yRange, yRange);
      v2_counts_withPtCut = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_rap_counts_Pt%g", fParticleName.at(pdg).c_str(), fPtThresholdForVs), "", nbins, -yRange, yRange);

      v1_withLEPtCut = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_LEPt%g_cumsum", fParticleName.at(pdg).c_str(), fPtThresholdForVs), "", nbins, -yRange, yRange);
      v2_withLEPtCut = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_LEPt%g_cumsum", fParticleName.at(pdg).c_str(), fPtThresholdForVs), "", nbins, -yRange, yRange);

      v1_counts_withLEPtCut = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v1_rap_counts_LEPt%g", fParticleName.at(pdg).c_str(), fPtThresholdForVs), "", nbins, -yRange, yRange);
      v2_counts_withLEPtCut = this -> RegisterHistogram<TH1F>(false, TString::Format("%s_v2_rap_counts_LEPt%g", fParticleName.at(pdg).c_str(), fPtThresholdForVs), "", nbins, -yRange, yRange);


    }

    auto particle = TDatabasePDG::Instance() -> GetParticle(pdg);
    double mass = (std::abs(pdg) == 211)? 1 :  particle -> Mass()/STAnaParticleDB::kAu2Gev;
    double Z = (std::abs(pdg) == 211)? 0 : particle -> Charge()/3.;

    this -> RegisterRuleWithParticle(pdg, 
      [this, pdg, CIP_v1, CIP_pt_v1, CIP_v2,
       v1, v2, v1_withPtCut, v2_withPtCut, 
       v1_withLEPtCut, v2_withLEPtCut, v1_counts_withLEPtCut, v2_counts_withLEPtCut,
       CIP_v1_counts, CIP_pt_v1_counts, CIP_v2_counts,
       v1_counts, v2_counts, 
       v1_counts_withPtCut, v2_counts_withPtCut, 
       v1_pt, v1_pt_counts, mass, Z](const DataPackage& package)
      {
        const auto& data = package.Data();
        for(int i = 0; i < data.multiplicity; ++i)
          if(std::fabs(package.StdDev(i)) < fSDThresholdForVs && 
          package.Prob(i) > fProbThresholdForVs && 
          package.PhiEff(i) > fPhiEffThresholdForVs && 
          (std::fabs(package.CMVector(i).Phi()*180/M_PI) < fPhiThresholdForVs) &&
	  data.recodpoca[i].Mag() < fMaxDPOCA)
          {
            double y0 = (fNNFrame)? package.LabRapidity(i)/(0.5*package.BeamRapidity()[1]) - 1 : package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);// package.FragRapidity(i)/(0.5*package.BeamRapidity()[1]);
            double phi = package.CMVector(i).Phi();
            double weight = package.Prob(i)/package.PhiEff(i);
            double pt = package.CMVector(i).Perp();

            if(!std::isnan(package.V1RPAngle(i)))
            {
              double cos1_weight = cos(phi - package.V1RPAngle(i))*weight;
              double cos2_weight = cos(2*(package.V1RPAngle(i) - phi))*weight;
              v1 -> Fill(y0, cos1_weight);
              v1_counts -> Fill(y0);

              CIP_v1 -> Fill(y0, cos1_weight*Z);
              CIP_v1_counts -> Fill(y0, Z);

              //phi = (phi < 0)? 2*TMath::Pi() + phi : phi; 
              v2 -> Fill(y0, cos2_weight);
              v2_counts -> Fill(y0);

              CIP_v2 -> Fill(y0, cos2_weight*Z);
              CIP_v2_counts -> Fill(y0, Z);

              if(fPtThresholdForVs > 0) 
              {
                if(pt/mass > fPtThresholdForVs)
                {
                  v1_withPtCut -> Fill(y0, cos1_weight);
                  v1_counts_withPtCut -> Fill(y0);

                  //_withPtCutphi = (phi < 0)? 2*TMath::Pi() + phi : phi; 
                  v2_withPtCut -> Fill(y0, cos2_weight);
                  v2_counts_withPtCut -> Fill(y0);
                }
                else
                {
                  v1_withLEPtCut -> Fill(y0, cos1_weight);
                  v1_counts_withLEPtCut -> Fill(y0);

                  //_withPtCutphi = (phi < 0)? 2*TMath::Pi() + phi : phi; 
                  v2_withLEPtCut -> Fill(y0, cos2_weight);
                  v2_counts_withLEPtCut -> Fill(y0);
                }
              }

              if(0.3 < y0 && y0 < ((std::abs(pdg) == 211)? 1.6 : 0.8)/*0.15 <  package.FragRapidity(i) &&  package.FragRapidity(i) < ((std::abs(pdg) == 211)? 0.9 : 0.45)*/)
              {
                v1_pt -> Fill(pt/mass, cos1_weight);
                v1_pt_counts -> Fill(pt/mass);

                CIP_pt_v1 -> Fill(pt/mass, cos1_weight*Z);
                CIP_pt_v1_counts -> Fill(pt/mass, Z);
              }
            }
          }
      });

    this -> RegisterFinishTaskRule([v1, v1_counts, v1_pt, v1_pt_counts, v2, v2_counts, 
                                    v1_withPtCut, v1_counts_withPtCut, 
                                    v2_withPtCut, v2_counts_withPtCut, 
                                    v1_withLEPtCut, v1_counts_withLEPtCut,
                                    v2_withLEPtCut, v2_counts_withLEPtCut, pdg, this]()
      {
        auto v1_cloned = (TH1F*) v1 -> Clone(TString::Format("%s_v1", fParticleName.at(pdg).c_str()));
        v1_cloned -> Divide(v1_counts);
        v1_cloned -> Write();

        auto v1_pt_cloned = (TH1F*) v1_pt -> Clone(TString::Format("%s_v1_Pt_MidRap", fParticleName.at(pdg).c_str()));
        v1_pt_cloned -> Divide(v1_pt_counts);
        v1_pt_cloned -> Write();

        auto v2_cloned = (TH1F*) v2 -> Clone(TString::Format("%s_v2", fParticleName.at(pdg).c_str()));
        v2_cloned -> Divide(v2_counts);
        v2_cloned -> Write();

        if(fPtThresholdForVs > 0)
        {
          v1_cloned = (TH1F*) v1_withPtCut -> Clone(TString::Format("%s_v1_Pt%g", fParticleName.at(pdg).c_str(), fPtThresholdForVs));
          v1_cloned -> Divide(v1_counts_withPtCut);
          v1_cloned -> Write();

          v2_cloned = (TH1F*) v2_withPtCut -> Clone(TString::Format("%s_v2_Pt%g", fParticleName.at(pdg).c_str(), fPtThresholdForVs));
          v2_cloned -> Divide(v2_counts_withPtCut);
          v2_cloned -> Write();

          v1_cloned = (TH1F*) v1_withLEPtCut -> Clone(TString::Format("%s_v1_LEPt%g", fParticleName.at(pdg).c_str(), fPtThresholdForVs));
          v1_cloned -> Divide(v1_counts_withLEPtCut);
          v1_cloned -> Write();

          v2_cloned = (TH1F*) v2_withLEPtCut -> Clone(TString::Format("%s_v2_LEPt%g", fParticleName.at(pdg).c_str(), fPtThresholdForVs));
          v2_cloned -> Divide(v2_counts_withLEPtCut);
          v2_cloned -> Write();

          int y0bin = v2_withPtCut -> GetXaxis() -> FindBin(double(0));
          int y0_5bin = v2_withPtCut -> GetXaxis() -> FindBin(0.5);
          TParameter<double>(TString::Format("%s_AveV2_Pt%g", fParticleName.at(pdg).c_str(), fPtThresholdForVs), v2_withPtCut->Integral(y0bin, y0_5bin)/v2_counts_withPtCut->Integral(y0bin, y0_5bin)).Write();
        }

        int y0bin = v2 -> GetXaxis() -> FindBin(double(0));
        int y0_5bin = v2 -> GetXaxis() -> FindBin(0.5);
        TParameter<double>(TString::Format("%s_AveV2", fParticleName.at(pdg).c_str()), v2->Integral(y0bin, y0_5bin)/v2_counts->Integral(y0bin, y0_5bin)).Write();
      });
  }

  this -> RegisterFinishTaskRule([CIP_v1, CIP_pt_v1, CIP_v2, CIP_v1_counts, CIP_pt_v1_counts, CIP_v2_counts, this]()
    {
      TParameter<double>("fMidRapThresholdForVs", fMidRapThresholdForVs).Write();
      TParameter<double>("PtThresholdForVs", fPtThresholdForVs).Write();
      TParameter<double>("ProbThresholdForVs", fProbThresholdForVs).Write();
      TParameter<double>("SDThresholdForVs", fSDThresholdForVs).Write();
      TParameter<double>("PhiEffThresholdForVs", fPhiEffThresholdForVs).Write();
      TParameter<double>("PhiThresholdForVs", fPhiThresholdForVs).Write();

      auto v1_cloned = (TH1F*) CIP_v1 -> Clone("CIP_v1");
      v1_cloned -> Divide(CIP_v1_counts);
      v1_cloned -> Write();

      auto v1_pt_cloned = (TH1F*) CIP_pt_v1 -> Clone("CIP_v1_Pt_MidRap");
      v1_pt_cloned -> Divide(CIP_pt_v1_counts);
      v1_pt_cloned -> Write();

      auto v2_cloned = (TH1F*) CIP_v2 -> Clone("CIP_v2");
      v2_cloned -> Divide(CIP_v2_counts);
      v2_cloned -> Write();
    });

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
  std::cout << "Beam energy = " << energyPerN << " AGeV" << std::endl;
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
            std::cout << "v1 for pdg " << *it << ": ";
            for(int paraType = 0; paraType < v1Npar; ++paraType) { 
              v1_para[paraType] = std::stof(PartInfo.at("v1_" + std::to_string(paraType)));
              std::cout << v1_para[paraType] << " ";
            }
            std::cout << std::endl;
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
          delta_phi = angEq.GetX(delta_phi, -TMath::Pi(), TMath::Pi(), 1e-5);
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
