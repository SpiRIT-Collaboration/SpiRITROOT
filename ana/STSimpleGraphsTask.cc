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

STSimpleGraphsTask::STSimpleGraphsTask() : fEntries(0)
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

InitStatus STSimpleGraphsTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  STAnaParticleDB::FillTDatabasePDG();
  auto db = TDatabasePDG::Instance();
  for(int pdg : fSupportedPDG)
  {
    auto pname = fParticleName[pdg];//db -> GetParticle(pdg) -> GetName();
    fHists.emplace_back(TString::Format("%s_ana", pname.c_str()),
                        ";y_{Lab}/y_{beam Lab};CM P_{t}/A (MeV/c2);",
                        60, 0, 1.5, 60, 0, 1000);
    fRapHists.emplace_back(TString::Format("%s_rapHist", pname.c_str()),
                           "", 500, -0.5, 1.5);
    fPtHists.emplace_back(TString::Format("%s_ptHist", pname.c_str()),
                          "", 500, -1.5, 1.5);
    fPtFullHists.emplace_back(TString::Format("%s_ptFullHist", pname.c_str()),
                              "", 500, -2, 2);


  }

  for(int pdg : fSupportedPDG)
  {
    fPIDForParticleHists.push_back(std::vector<std::vector<TH2F*>>(fNPitches, std::vector<TH2F*>(fNYaw, nullptr)));
    for(int i = 0; i < fNPitches; ++i)
      for(int j = 0; j < fNYaw; ++j)
        fPIDForParticleHists.back()[i][j] = new TH2F(TString::Format("Pitch%dYaw%dPDG%d", i, j, pdg), "", 
                                                     fMomBins, fMinMom, fMaxMom, fdEdXBins, fMindEdX, fMaxdEdX);
  }


  //fPDG = (STVectorI*) ioMan -> GetObject("PDG");
  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  fEff = (TClonesArray*) ioMan -> GetObject("Eff");
  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");
  fLabRapidity = (TClonesArray*) ioMan -> GetObject("LabRapidity");
  fBeamRapidity = (STVectorF*) ioMan -> GetObject("BeamRapidity");
  fFragVelocity = (TClonesArray*) ioMan -> GetObject("FragVelocity");
  fFragRapidity = (TClonesArray*) ioMan -> GetObject("FragRapidity");
  
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
  auto data = (STData*) fData -> At(0);
  if(fabs(data -> beamEnergy - 270) < 5)
  {
    int npart = data -> multiplicity;
    auto& beamRapidity = fBeamRapidity -> fElements;
     
    for(int i = 0; i < fSupportedPDG.size(); ++i)
    {
      auto& cmVector = static_cast<STVectorVec3*>(fCMVector -> At(i)) -> fElements;
      auto& labRapidity = static_cast<STVectorF*>(fLabRapidity -> At(i)) -> fElements;
      auto& eff = static_cast<STVectorF*>(fEff -> At(i)) -> fElements;
      auto& prob = static_cast<STVectorF*>(fProb -> At(i)) -> fElements;
      auto& fragRapidity = static_cast<STVectorF*>(fFragRapidity -> At(i)) -> fElements;
      auto& fragVelocity = static_cast<STVectorVec3*>(fFragVelocity -> At(i)) -> fElements;
      int ipdg = fSupportedPDG[i];
      auto particle = TDatabasePDG::Instance() -> GetParticle(ipdg);
      auto minMom = fMinMomForCMInLab[ipdg];
      for(int partid = 0; partid < npart; ++partid)
      {
        int pitchID = this -> _ToPitchId(data -> vaMom[partid]);
        int yawId = this -> _ToYawId(data -> vaMom[partid]);
        if(data -> vaNRowClusters[partid] + data -> vaNLayerClusters[partid] > fMinNClus && data -> recodpoca[partid].Mag() < fMaxDPOCA)
          fPIDForParticleHists[i][pitchID][yawId] -> Fill(data -> vaMom[partid].Mag(), data -> vadedx[partid], prob[partid]);
        if(eff[partid] > 0.10 && prob[partid] > 0.)
        {
          double weight = prob[partid]/eff[partid];
          if(data -> vaMom[partid].Mag() > minMom) 
            fHists[i].Fill(labRapidity[partid]/beamRapidity[1], cmVector[partid].Perp()/(particle -> Mass()/STAnaParticleDB::kAu2Gev), weight);
          if(fragRapidity[partid] > 0) 
          {
            double cosphi = cos(gRandom -> Uniform(-TMath::Pi(), TMath::Pi()));
            double perp = fragVelocity[i].Perp();
            double perp_cosphi = cosphi*perp;
            double ptRap = 0.5*log((1+perp_cosphi)/(1-perp_cosphi));
            fPtFullHists[i].Fill(ptRap/beamRapidity[0], weight);
            if(fragRapidity[partid]/beamRapidity[1] < 0.5) fPtHists[i].Fill(ptRap/beamRapidity[0], weight);
          }
          fRapHists[i].Fill(fragRapidity[partid]/beamRapidity[0], weight);
        }
      }
    }
    ++fEntries;
  }
}

void STSimpleGraphsTask::FinishTask()
{
  auto outfile = FairRootManager::Instance() -> GetOutFile();
  outfile -> cd();

  auto Set1DUnit = [this](TH1& hist) -> TH1&
  {
    auto xaxis = hist.GetXaxis();
    double range = xaxis->GetXmax() - xaxis->GetXmin();
    double binSize = xaxis->GetBinWidth(0);
    hist.Scale(range/binSize/fEntries);
    return hist;
  };

  for(auto& hist : fPtHists) Set1DUnit(hist).Write(); 
  for(auto& hist : fPtFullHists) Set1DUnit(hist).Write(); 
  for(auto& hist : fRapHists) Set1DUnit(hist).Write(); 

  std::map<int, TH1D*> ptHists;
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    auto& hist = fHists[i];
    auto HistName = hist.GetName();
    auto projx = hist.ProjectionX(TString::Format("%s_Rap", HistName));
    auto projy = hist.ProjectionY(TString::Format("%s_Pt", HistName),
                                 hist.GetXaxis() -> FindBin(double(0.5)), 
                                 hist.GetXaxis() -> FindBin(double(0.6)));
    projy -> Rebin();
    Set1DUnit(*projy).Write();

    projx -> Rebin();
    Set1DUnit(*projx);
    projx -> Write();
    ptHists[fSupportedPDG[i]] = projy;

    hist.Scale(1./fEntries);
    hist.Write();
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
    cloned -> Scale(Z);
    CIP -> Add(cloned);

    cloned = (TH1D*) key.second -> Clone(TString::Format("%s_ClonedForN", key.second -> GetName()));
    cloned -> Scale(A - Z);
    CIN -> Add(cloned); 
  }

  CIP -> Write();
  CIN -> Write();

  CIN -> Divide(CIP);
  CIN -> Write("CINCIP_ana_Pt");

  gROOT -> SetBatch(true);
  std::vector<std::vector<TH2F*>> PIDAllHists(fNPitches, std::vector<TH2F*>(fNYaw, nullptr));
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    auto& PIDHists = fPIDForParticleHists[i];
    TCanvas c1;
    OrgainzePIDs(&c1, PIDHists, fNYaw, fNPitches);
    c1.Write(TString::Format("PIDCanvas%d", fSupportedPDG[i]));

    for(int pitchId = 0; pitchId < fNPitches; ++pitchId)
      for(int yawId = 0; yawId < fNYaw; ++yawId)
        if(PIDAllHists[pitchId][yawId]) PIDAllHists[pitchId][yawId] -> Add(PIDHists[pitchId][yawId]);
        else PIDAllHists[pitchId][yawId] = (TH2F*) PIDHists[pitchId][yawId] -> Clone(TString::Format("Pitch%dYaw%d", pitchId, yawId));
  }

  TCanvas c1;
  OrgainzePIDs(&c1, PIDAllHists, fNYaw, fNPitches);
  c1.Write("PIDCanvas");
  gROOT -> SetBatch(false);
    
}

void STSimpleGraphsTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }

