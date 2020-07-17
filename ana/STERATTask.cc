#include "STERATTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TRandom.h"
#include "STVector.hh"
#include "TDatabasePDG.h"

#include <cmath>

ClassImp(STERATTask);

STERATTask::STERATTask()
{
  fLogger = FairLogger::GetLogger(); 
  fbERat = new STVectorF();
  fbERat -> fElements.push_back(0);
  fbMult = new STVectorF();
  fbMult -> fElements.push_back(0);
  fERAT = new STVectorF();
  fERAT -> fElements.push_back(0);
}

STERATTask::~STERATTask()
{}

InitStatus STERATTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  ioMan -> Register("ERAT", "ST", fERAT, fIsPersistence);

  if(!fImpactParameterFilename.empty())
  {
    fLogger -> Info(MESSAGE_ORIGIN, ("Loading impact parameter distribution from " + fImpactParameterFilename).c_str());
    fImpactParameterFile = new TFile(fImpactParameterFilename.c_str());
    if(!fImpactParameterFile -> IsOpen())
    {
      fLogger -> Info(MESSAGE_ORIGIN, (fImpactParameterFilename + " cannot be opened. Will not calculate impact parameter").c_str());
      fImpactParameterFile -> Delete();
      fImpactParameterFile = nullptr;
    }
    else
    {
      fMultHist = (TH1F*) fImpactParameterFile -> Get("Mult");
      fERatHist = (TH1F*) fImpactParameterFile -> Get("ERat");
      fData = (TClonesArray*) ioMan -> GetObject("STData");
      ioMan -> Register("bERat", "ST", fbERat, fIsPersistence);
      ioMan -> Register("bMult", "ST", fbMult, fIsPersistence);
    }
  }

  return kSUCCESS;
}

void STERATTask::SetParContainers()
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

void STERATTask::Exec(Option_t *opt)
{
  double Et_expected = 0;
  double El_expected = 0;
  for(int i = 0; i < fSupportedPDG.size(); ++i)
    if(auto particle = TDatabasePDG::Instance() -> GetParticle(fSupportedPDG[i]))
    {
      double ParticleMass = particle -> Mass()*1000;
      int ntracks = static_cast<STVectorF*>(fProb -> At(i)) -> fElements.size();
      for(int itrack = 0; itrack < ntracks; ++itrack)
      {
        auto& P = static_cast<STVectorVec3*>(fCMVector -> At(i)) -> fElements[itrack];
        if(P.z() < 0) continue;
        double prob = static_cast<STVectorF*>(fProb -> At(i)) -> fElements[itrack];
        double Ei = sqrt(ParticleMass*ParticleMass + P.Mag2());
        double pt = P.Perp();
        double pl = P.z();
        Et_expected += prob*pt*pt/(ParticleMass + Ei);
        El_expected += prob*pl*pl/(ParticleMass + Ei);
      }
    }
  fERAT -> fElements[0] = Et_expected / El_expected;

  if(fImpactParameterFile)
  {
    int mult = 0;
    auto data = static_cast<STData*>(fData -> At(0));
    for(int i = 0; i < data -> multiplicity; ++i) 
      if(data -> recodpoca[i].Mag() < 20) 
        ++mult;
    fbMult -> fElements[0] = fMultHist -> Interpolate(mult);
    fbERat -> fElements[0] = fERatHist -> Interpolate(fERAT -> fElements[0]);
  }
}

void STERATTask::SetImpactParameterTable(const std::string& table_filename)
{
  fImpactParameterFilename = table_filename;
}

void STERATTask::CreateImpactParameterTable(const std::string& ana_filename, const std::string& output_filename)
{
  const int fMultMin = 0;
  const int fMultMax = 500;
  const double fERatMin = 0;
  const double fERatMax = 5;
  const int fERatBins = 300;

  TChain chain("cbmsim");
  chain.Add(ana_filename.c_str());

  TFile output(output_filename.c_str(), "RECREATE");
  TH1F mult_hist("Mult", "Mult", fMultMax - fMultMin + 1, fMultMin, fMultMax);
  chain.Project("Mult", "Sum$(recodpoca.Mag() < 20)");
  auto cumulative = mult_hist.GetCumulative(false);
  int max = double(cumulative -> GetMaximum());
  for(int i = 1; i < cumulative -> GetNbinsX(); ++i)
    cumulative -> SetBinContent(i, sqrt(cumulative -> GetBinContent(i)/max));
  output.cd();
  cumulative -> Write("Mult");

  TH1F erat_hist("erat", "erat", fERatBins, fERatMin, fERatMax);
  chain.Project("erat", "ERAT.fElements");
  cumulative = erat_hist.GetCumulative(false);
  max = double(cumulative -> GetMaximum());
  for(int i = 1; i < cumulative -> GetNbinsX(); ++i)
    cumulative -> SetBinContent(i, sqrt(cumulative -> GetBinContent(i)/max));
  output.cd();
  cumulative -> Write("ERat");
}
