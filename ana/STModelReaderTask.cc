#include "STModelReaderTask.hh"

// FAIRROOT classes
#include "FairRootManager.h"
#include "FairRunOnline.h"
#include "FairRun.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include <glob.h>
#include "TXMLAttr.h"
#include "TRandom.h"

ClassImp(STModelReaderTask);

STModelReaderTask::STModelReaderTask(TString filename, bool enable_neutrons) : fEnableNeutrons(enable_neutrons)
{
  STAnaParticleDB::EnableChargedParticles();
  STAnaParticleDB::EnablePions();
  if(enable_neutrons) STAnaParticleDB::EnableMCNeutrons();
  fSupportedPDG = STAnaParticleDB::GetSupportedPDG();

  fLogger = FairLogger::GetLogger(); 

  fData = new TClonesArray("STData");
  fProb = new TClonesArray("STVectorF");
  fEff = new TClonesArray("STVectorF");

  auto fInputPath = TString::Format("%s/input/", gSystem->Getenv("VMCWORKDIR"));
  fReader = std::unique_ptr<STTransportReader>(ReaderFactory(fInputPath, filename));
  if(!fReader)
    LOG(FATAL)<<"STModelReader cannot accept event files without specifying generator names.\nInput name : " << filename << FairLogger::endl;

  fEventID = new TClonesArray("STVectorI");
  fRunID = new TClonesArray("STVectorI");
}

STModelReaderTask::~STModelReaderTask()
{}

int STModelReaderTask::GetNEntries()
{ return fReader -> GetEntries(); }

InitStatus STModelReaderTask::Init()
{
  STAnaParticleDB::FillTDatabasePDG();
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }
 
  new((*fData)[0]) STData();
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    new((*fProb)[i]) STVectorF();
    new((*fEff)[i]) STVectorF();
  }

  (new((*fEventID)[0]) STVectorI()) -> fElements.push_back(0);
  (new((*fRunID)[0]) STVectorI()) -> fElements.push_back(0);

  ioMan -> Register("STData", "ST", fData, fIsPersistence);
  ioMan -> Register("Prob", "ST", fProb, fIsPersistence);
  ioMan -> Register("Eff", "ST", fEff, fIsPersistence);
  ioMan -> Register("EventID", "ST", fEventID, fIsPersistence);
  ioMan -> Register("RunID", "ST", fRunID, fIsPersistence);

  if(fRotate)
    fLogger -> Info(MESSAGE_ORIGIN, "The event will be randomly rotated along the beam axis");
  else
    fLogger -> Info(MESSAGE_ORIGIN, "The event will not be randomly rotated");

  fMCRotZ = new STVectorF();
  fMCRotZ -> fElements.push_back(0);
  ioMan -> Register("MCRotZ", "ST", fMCRotZ, fIsPersistence);
  return kSUCCESS;
}

void
STModelReaderTask::SetParContainers()
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

void STModelReaderTask::Exec(Option_t *opt)
{
  std::vector<STTransportParticle> particleList;
  if(fReader -> GetNext(particleList))
  {
    fLogger -> Info(MESSAGE_ORIGIN, TString::Format("Event %d", fReader -> GetEntry()));
    auto data = static_cast<STData*>(fData -> At(0));
    data -> ResetDefaultWithLength(particleList.size());
    data -> beamEnergyTargetPlane = fEnergyPerA;
    data -> z = 1.;
    data -> aoq = fBeamA;
    data -> proja = data -> projb = 0;

    if(fRotate)
    {
      double phi = gRandom -> Uniform(-TMath::Pi(), TMath::Pi());
      fMCRotZ -> fElements[0] = phi;
    }

    for(int i = 0; i < data -> multiplicity; ++i)
    {
      const auto& particle = particleList[i];
      auto p_info = TDatabasePDG::Instance() -> GetParticle(particle.pdg);
      if(!p_info) continue;
      double Z = p_info -> Charge()/3.;
      if(Z == 0) 
      {
        if(fEnableNeutrons)
        {
          if(particle.pdg == 2112)
            Z = 1;
          else
            continue; // neutral particles cannot be detected
        }
        else continue;
      }
      TLorentzVector fragVect;
      fragVect.SetXYZM(particle.px, particle.py, particle.pz, p_info -> Mass());
      fragVect.Boost(fBoostVector); 
      if(fRotate) fragVect.RotateZ(fMCRotZ -> fElements[0]);

      data -> recodpoca[i].SetXYZ(0, 0, 0);
      data -> recoNRowClusters[i] = 100;
      data -> recoNLayerClusters[i] = 100; 
      data -> recoMom[i].SetXYZ(fragVect.Px()*1000/std::fabs(Z), fragVect.Py()*1000/std::fabs(Z), fragVect.Pz()*1000/std::fabs(Z));
      data -> recoCharge[i] = (Z > 0) ? 1 : -1;

      data -> vaMom[i] = data -> recoMom[i];
      data -> vaNRowClusters[i] = 100;
      data -> vaNLayerClusters[i] = 100;
      data -> vaCharge[i] = data -> recoCharge[i];
    }

    for(int i = 0; i < fSupportedPDG.size(); ++i)
    {
      auto& prob = static_cast<STVectorF*>(fProb -> At(i)) -> fElements;
      prob.clear();
      auto& eff = static_cast<STVectorF*>(fEff -> At(i)) -> fElements;
      eff.clear();

      for(const auto& particle : particleList)
      {
        if(fSupportedPDG[i] == particle.pdg)
        {
          prob.push_back(1);
          eff.push_back(1);
        }else
        {
          prob.push_back(0);
          eff.push_back(0);
        }
      }
    }
    ++static_cast<STVectorI*>(fEventID -> At(0)) -> fElements[0];
  }else fLogger -> Fatal(MESSAGE_ORIGIN, "Event ID exceeds the length of the TChain");
}

void STModelReaderTask::SetPersistence(Bool_t value)                                              
{ fIsPersistence = value; }
void STModelReaderTask::SetEventID(int eventID)
{ fReader -> SetEntry(eventID); }
void STModelReaderTask::SetBeamAndTarget(int beamA, int targetA, double energyPerA)
{ 
  fTargetA = targetA; 
  fBeamA = beamA; 
  fEnergyPerA = energyPerA;

  double E = energyPerA*beamA + beamA*931.5;
  double pMag = std::sqrt(E*E - beamA*beamA*931.5*931.5);
  fFourVect.SetPxPyPzE(0, 0, pMag, E + targetA*931.5);
  fBoostVector = fFourVect.BoostVector();
}

