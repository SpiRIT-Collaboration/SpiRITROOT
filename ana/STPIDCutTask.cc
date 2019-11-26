#include "STPIDCutTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "STVector.hh"

ClassImp(STPIDCutTask);

STPIDCutTask::STPIDCutTask()
{ 
  fLogger = FairLogger::GetLogger(); 
  fPDGLists = new TClonesArray("STVectorI");
  fMassLists = new TClonesArray("STVectorF");
}

STPIDCutTask::~STPIDCutTask()
{}

InitStatus STPIDCutTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  for(int i = 0; i < fNPitches; ++i)
  {
    std::vector<TH2F*> yawHists;
    for(int j = 0; j < fNYaw; ++j)
      yawHists.push_back(new TH2F(TString::Format("Pitch%dYaw%d", i, j), "PID", 
                                  fMomBins, fMinMom, fMaxMom, fdEdXBins, fMindEdX, fMaxdEdX));
    fPIDHists.push_back(yawHists);
  }

  auto namelist = ioMan -> GetBranchNameList();
  fData = (TClonesArray*) ioMan -> GetObject("STData");
  ioMan -> Register("PDG", "ST", fPDGLists, fIsPersistence);
  ioMan -> Register("Mass", "ST", fMassLists, fIsPersistence);
  return kSUCCESS;
}

void
STPIDCutTask::SetParContainers()
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

void STPIDCutTask::Exec(Option_t *opt)
{
  fPDGLists -> Delete();
  auto data = (STData*) fData -> At(0);
  int npart = data -> multiplicity;

  auto pdg_ary = new((*fPDGLists)[0]) STVectorI;
  auto mass_ary = new((*fMassLists)[0]) STVectorF;

  for(int part = 0; part < npart; ++part)
  {
    int nClus = data -> vaNRowClusters[part] + data -> vaNLayerClusters[part];
    double poca = data ->recodpoca[part].Mag();
    const auto& mom = data -> vaMom[part];
    const auto& charge = data -> vaCharge[part];
    const auto& dedx = data -> vadedx[part];

    double momMag = mom.Mag()/charge;
    int pdg = 0;
    double mass = 0;
    for(int icut = 0; icut < fCuts.size(); ++icut)
      if(fCuts[icut] -> IsInside(momMag, dedx))
      { 
        pdg = fPDG[icut];
        mass = fMass[icut];
        break;
      }
    pdg_ary -> fElements.push_back(pdg); 
    mass_ary -> fElements.push_back(mass);
 
    if(nClus > fMinNClus && poca < fMaxDPOCA)
    {
      int pitchId = this->_ToPitchId(mom);
      int yawId = this->_ToYawId(mom);

      if(pitchId >= 0 && yawId >= 0)
        fPIDHists[pitchId][yawId] -> Fill(momMag, dedx);
    }
  }
}

void STPIDCutTask::FinishTask()
{
  FairRootManager::Instance() -> GetOutFile();
  for(auto& yaws : fPIDHists)
    for(auto hist : yaws)
      hist -> Write();
}

void STPIDCutTask::SetPersistence(Bool_t value)                                              { fIsPersistence = value; }
void STPIDCutTask::SetNPitch(int pitches)                                                  { fNPitches = pitches; }
void STPIDCutTask::SetNYaw(int yaws)                                                         { fNYaw = yaws; }
void STPIDCutTask::SetCutConditions(int minNClus, double maxPOCA)                            { fMinNClus = minNClus; fMaxDPOCA = maxPOCA; }
void STPIDCutTask::SetCutFile(const std::string& cutfile)
{
  fCutFile = new TFile(cutfile.c_str());
  if(!fCutFile -> IsOpen())
  {
    fLogger -> Error(MESSAGE_ORIGIN, TString::Format("Cutfile %s cannot be opened", cutfile.c_str()));
    return;
  }
  else fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Loading cuts from file %s", cutfile.c_str()));

  for(const auto& particle : fParticleNames)
  {
    auto cutg = static_cast<TCutG*>(fCutFile -> Get(particle.c_str()));
    if(cutg) 
      fCuts.push_back(static_cast<TCutG*>(fCutFile -> Get(particle.c_str())));
    else
    {
      fCuts.push_back(nullptr);
      fLogger -> Error(MESSAGE_ORIGIN, TString::Format("Particle cut for %s is not found", particle.c_str())); 
    }
  }
}


int STPIDCutTask::_ToPitchId(const TVector3& vec)
{
  double pitch = TMath::ATan2(vec.y(), sqrt(vec.x()*vec.x() + vec.z()*vec.z()));
  double dPitch = TMath::Pi()/fNPitches;
  if(std::fabs(pitch) < TMath::Pi()/2.)
    return (pitch + TMath::Pi()/2.)/dPitch;
  else
    return -1;
}

int STPIDCutTask::_ToYawId(const TVector3& vec)
{
  double yaw = TMath::ATan2(vec.x(), vec.z());
  double dYaw = TMath::Pi()/fNYaw;
  if(std::fabs(yaw) < TMath::Pi()/2.)
    return (yaw + TMath::Pi()/2.)/dYaw;
  else
    return -1;
}

