#include "STObsWriterTask.hh"

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

ClassImp(STObsWriterTask);

STObsWriterTask::STObsWriterTask(const std::string& output_name) : fOutput(output_name)
{
  fLogger = FairLogger::GetLogger(); 
  fProtonMass = TDatabasePDG::Instance() -> GetParticle(2212) -> Mass()*1000;
}

STObsWriterTask::~STObsWriterTask()
{}

void STObsWriterTask::LoadTrueImpactPara(const std::string& filename)
{
  fUrQMDReader = new STUrQMDReader(filename);
}

InitStatus STObsWriterTask::Init()
{
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fData = (TClonesArray*) ioMan -> GetObject("STData");
  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");
  fProb = (TClonesArray*) ioMan -> GetObject("Prob");
  fCMVector = (TClonesArray*) ioMan -> GetObject("CMVector");
  fERAT = (STVectorF*) ioMan -> GetObject("ERAT");
  fET = (STVectorF*) ioMan -> GetObject("ET");
  fFragRapidity = (TClonesArray*) ioMan -> GetObject("FragRapidity");
  fBeamRapidity = (STVectorF*) ioMan -> GetObject("BeamRapidity");
  fSkip = (STVectorI*) ioMan -> GetObject("Skip");

  if(!fET || !fERAT)
  {
    fLogger -> Error(MESSAGE_ORIGIN, "No transverse momentum and ERat found. Have you added STERATTask before STObsWriterTask?");
    return kERROR;
  }

  fAllObs = new STVectorF;
  fAllObs -> fElements.resize(static_cast<int>(ObsType::END));
  ioMan -> Register("AllObs", "ST", fAllObs, fIsPersistence);

  for(int i = 0; i < static_cast<int>(ObsType::END); ++i)
    fOutput << ((i == 0)? "" : "\t") << ObsHeader[i];

  if(fUrQMDReader)
  {
    fImpactParameterTruth = new STVectorF();
    fImpactParameterTruth -> fElements.push_back(0);
    ioMan -> Register("bTruth", "ST", fImpactParameterTruth, fIsPersistence);
    fEventID = (TClonesArray*) ioMan -> GetObject("EventID");
    fOutput << "\tbim";
  }
  fOutput << std::endl;

  return kSUCCESS;
}

void STObsWriterTask::SetParContainers()
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

void STObsWriterTask::Exec(Option_t *opt)
{
  if(fSkip)
    if(static_cast<STVectorI*>(fSkip) -> fElements[0] == 1) return;

  double et = fET -> fElements[0];
  double erat = fERAT -> fElements[0];
  double Mch = 0; // multiplicity
  double N_H_He = 0; // tot number of H and He isotopes
  double N_H_He_pt = 0; // averaged transverse mom of H and He
  double N = 0; // num of free protons at mid-rapidity
  double Npt = 0; // averaged transverse momentum of free proton at mid-rapidity

  auto data = static_cast<STData*>(fData -> At(0));
  for(int i = 0; i < data -> multiplicity; ++i)
    if(data -> recodpoca[i].Mag() < 20)
      Mch += 1.;

  // calculate proton ERAT 
  double Et_proton = 0, El_proton = 0;
  double N_H_He_ptNoP = 0;
  double N_H_He_NoP = 0;
  double Et_INDRA = 0; // Et in INDRA in PRC55.1906

  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    int ntracks = static_cast<STVectorF*>(fProb -> At(i)) -> fElements.size();
    for(int itrack = 0; itrack < ntracks; ++itrack)
    {
      double prob = static_cast<STVectorF*>(fProb -> At(i)) -> fElements[itrack];
      double rap = static_cast<STVectorF*>(fFragRapidity -> At(i)) -> fElements[itrack];
      if(rap > 0)
      {
        if(data -> recodpoca[itrack].Mag() < 20 /*&& data -> vaNRowClusters[itrack] + data -> vaNLayerClusters[itrack] > 8*/ && prob > /*0.2*/0.4)
        {
          auto& P = static_cast<STVectorVec3*>(fCMVector -> At(i)) -> fElements[itrack];
          double pt = P.Perp();
          double pl = P.z();

          if(fSupportedPDG[i] == 2212) 
          {
            double Ei = sqrt(fProtonMass*fProtonMass + P.Mag2());
            Et_proton += prob*pt*pt/(fProtonMass + Ei);
            El_proton += prob*pl*pl/(fProtonMass + Ei);
          }
          if(fSupportedPDG[i] == 2212 || fSupportedPDG[i] == 1000010020 || fSupportedPDG[i] == 1000010030 || fSupportedPDG[i] == 1000020030 || fSupportedPDG[i] == 1000020040 || fSupportedPDG[i] == 1000020060)
          {
            if(fSupportedPDG[i] != 2212) 
            {
              N_H_He_NoP += prob;
              N_H_He_ptNoP += (pt*prob)/((fSupportedPDG[i] % 10000)/10); // per nucleon
            }
            N_H_He += prob;
            N_H_He_pt += (pt*prob);
            Et_INDRA += pt*pt*prob/(2*fProtonMass*(fSupportedPDG[i] % 10000)/10);
            if(fSupportedPDG[i] == 2212 && rap/(0.5*fBeamRapidity -> fElements[1]) < 0.5)
            {
              N += prob;
              Npt += (P.Perp()*prob);
            }
          }
        }
      }
    } 
  }
  if(N_H_He_NoP > 0) N_H_He_ptNoP /= N_H_He_NoP;
  if(N_H_He > 0) N_H_He_pt /= N_H_He;
  if(N > 0) Npt /= N;

  if(fUrQMDReader)
  {
    double b = -9999;
    int id = static_cast<STVectorI*>(fEventID -> At(0)) -> fElements[0];
    if(fUrQMDReader) 
    {
      fUrQMDReader -> SetEntry(id - 1);
      b = fUrQMDReader -> GetB();
    }
    fImpactParameterTruth -> fElements[0] = b;
  }


  fAllObs -> fElements[static_cast<int>(ObsType::ET)] = et;
  fAllObs -> fElements[static_cast<int>(ObsType::ERat)] = erat;
  fAllObs -> fElements[static_cast<int>(ObsType::MCh)] = Mch;
  fAllObs -> fElements[static_cast<int>(ObsType::N_H_He)] =  N_H_He;
  fAllObs -> fElements[static_cast<int>(ObsType::N_H_He_pt)] = N_H_He_pt;
  fAllObs -> fElements[static_cast<int>(ObsType::N)] = N;
  fAllObs -> fElements[static_cast<int>(ObsType::Npt)] = Npt;
  double eratP = (El_proton != 0)? Et_proton/El_proton : 0;
  fAllObs -> fElements[static_cast<int>(ObsType::ERatOnlyP)] = eratP;
  fAllObs -> fElements[static_cast<int>(ObsType::N_H_He_NoP)] = N_H_He_NoP;
  fAllObs -> fElements[static_cast<int>(ObsType::N_H_He_ptNoP)] = N_H_He_ptNoP;
  fAllObs -> fElements[static_cast<int>(ObsType::ET_alt)] = Et_INDRA;

  for(int i = 0; i < static_cast<int>(ObsType::END); ++i)
    fOutput << ((i == 0)? "" : "\t") << fAllObs -> fElements[i];
  if(fUrQMDReader) fOutput << "\t" << fImpactParameterTruth -> fElements[0];
  fOutput << std::endl;
}