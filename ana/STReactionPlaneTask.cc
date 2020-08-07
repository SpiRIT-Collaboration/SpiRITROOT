#include "STReactionPlaneTask.hh"

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
#include "TH2.h"

#include <cmath>

ClassImp(STReactionPlaneTask);

STReactionPlaneTask::STReactionPlaneTask()
{
  fLogger = FairLogger::GetLogger(); 
  fReactionPlane = new STVectorF();
  fReactionPlane -> fElements.push_back(0);
}

STReactionPlaneTask::~STReactionPlaneTask()
{}

InitStatus STReactionPlaneTask::Init()
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
    fEff = (TH1F*) fEffFile -> Get("PhiEfficiency");
    fEff -> Smooth();
    if(!fEff) fLogger -> Info(MESSAGE_ORIGIN, "Efficiency histogram cannot be loaded. Will ignore efficiency");
  }
  fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Theta cut for reaction plane < %f deg", fThetaCut));
  fThetaCut = fThetaCut*TMath::DegToRad();

  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  fFragRapidity = (TClonesArray*) ioMan -> GetObject("FragRapidity");
  fBeamRapidity = (STVectorF*) ioMan -> GetObject("BeamRapidity");
  fMCRotZ = (STVectorF*) ioMan -> GetObject("MCRotZ");

  if(fUseMCReactionPlane)
  {
    if(fMCRotZ) fLogger -> Info(MESSAGE_ORIGIN, "Rotate particles according to the MC truth reaction plane.");
    else
    {
      fLogger -> Info(MESSAGE_ORIGIN, "UseMCReactionPlane is set but cannot load MC truth reaction plane. Will use inferred reaction plane instead");
      fUseMCReactionPlane = false;
    }
  }

  ioMan -> Register("RP", "ST", fReactionPlane, fIsPersistence);

  return kSUCCESS;
}

void STReactionPlaneTask::SetParContainers()
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

void STReactionPlaneTask::Exec(Option_t *opt)
{
  // use q vector method. Assume no efficiency problem
  TVector2 Q(0,0);
 
  std::vector<std::vector<TVector2>> Q_elements(fSupportedPDG.size());
  auto beamRap = fBeamRapidity -> fElements[1];
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    auto cmVector = static_cast<STVectorVec3*>(fCMVector -> At(i));
    auto prob = static_cast<STVectorF*>(fProb -> At(i));
    auto rap = static_cast<STVectorF*>(fFragRapidity -> At(i));
    int mult = cmVector -> fElements.size();
    auto pinfo = TDatabasePDG::Instance() -> GetParticle(fSupportedPDG[i]);
    auto mass = pinfo -> Mass()/STAnaParticleDB::kAu2Gev;
    auto charge = pinfo -> Charge()/3.;

    for(int j = 0; j < mult; ++j)
    {
      if(fabs(2*rap -> fElements[j]/beamRap) > fMidRapidity) // reject mid-rapidity particles
      {
        const auto& vec = cmVector -> fElements[j];
        if(vec.Theta() < fThetaCut)
        {
          double prob_ele = prob -> fElements[j];
          double eff = 1;
          if(fEff) eff = fEff -> Interpolate(vec.Theta(), vec.Phi());
          double weight = prob_ele/eff;

          // coefficients of the generalized Q-vector method
          if(fParticleCoef.size() == fSupportedPDG.size()) weight *= fParticleCoef[i];
          else weight *= fChargeCoef*charge + fMassCoef*mass + fConstCoef;

          // front size pos, back side neg
          if(vec.z() > 0) Q_elements[i].push_back(weight*vec.XYvector());
          else Q_elements[i].push_back(-weight*vec.XYvector());
          Q += Q_elements[i].back();
        }
        else Q_elements[i].emplace_back(0, 0);
      }
      else Q_elements[i].emplace_back(0, 0);
    }
  }
  if(sqrt(Q.X()*Q.X() + Q.Y()*Q.Y()) > 0) 
  {
    auto phi = Q.Phi();
    fReactionPlane -> fElements[0] = (phi > TMath::Pi())? phi - 2*TMath::Pi() : phi;
  }
  else fReactionPlane -> fElements[0] = -9999;

  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    auto cmVector = static_cast<STVectorVec3*>(fCMVector -> At(i));
    int mult = cmVector -> fElements.size();
    for(int j = 0; j < mult; ++j)
    {
      // eliminate correlation by removing the particle of interest constribution to reaction plane determination
      double phi = (fUseMCReactionPlane)? fMCRotZ -> fElements[0] : (Q - Q_elements[i][j]).Phi();
      cmVector -> fElements[j].RotateZ(-phi);
    }
  }
}

void STReactionPlaneTask::LoadPhiEff(const std::string& eff_filename)
{ fEffFilename = eff_filename; }

void STReactionPlaneTask::CreatePhiEffFromData(const std::string& ana_filename, const std::string& out_filename)
{
  TChain chain("cbmsim");
  chain.Add(ana_filename.c_str());

  // check if the momentum vector has been rotated
  // can only create efficiency if the data is NOT rotated
  {
    auto fileList = chain.GetListOfFiles();
    for(int i = 0; i < fileList->GetEntries(); ++i)
    {
      auto filename = fileList -> At(i) -> GetTitle();
      TFile file(filename);
      auto taskList = static_cast<FairFileHeader*>(file.Get("FileHeader")) -> GetListOfTasks();
      if(taskList -> FindObject("STReactionPlaneTask"))
      {
        std::cerr << "STReactionPlaneTask is found in " << filename << std::endl; 
        std::cerr << "This means the even has been rotated. Phi efficiency can only be extracted from files WITHOUT rotation.\n";
        std::cerr << "ABORT. Please try again for files that is not processed by STReactionPlaneTask.\n";
        return;
      }
    }
  }
      
  TH2F hist("PhiEfficiency", "", 40, 0, TMath::Pi(), 20, -TMath::Pi(), TMath::Pi());
  chain.Project("PhiEfficiency", "CMVector[0].fElements.Phi():CMVector[0].fElements.Theta()", "Prob[0].fElements*(Prob[0].fElements > 0.2)");
  
  // normalize
  hist.Smooth();
  for(int i = 0; i <= hist.GetNbinsX(); ++i)
  {
    // normalize each x-bin
    auto proj = hist.ProjectionY("_px", i, i);
    double max = proj -> GetMaximum();
    proj -> Delete();
    for(int j = 0; j <= hist.GetNbinsY(); ++j) 
      hist.SetBinContent(i, j, hist.GetBinContent(i, j)/max);
  }
  TFile output(out_filename.c_str(), "RECREATE");
  hist.Write();
}
