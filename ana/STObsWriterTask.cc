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
  fOutput << "ET\tERat\tMCh\tN(H,He)\tN(H,He)pt\tN\tNpt\n";
}

STObsWriterTask::~STObsWriterTask()
{}

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

  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    int ntracks = static_cast<STVectorF*>(fProb -> At(i)) -> fElements.size();
    for(int itrack = 0; itrack < ntracks; ++itrack)
    {
      double prob = static_cast<STVectorF*>(fProb -> At(i)) -> fElements[itrack];
      double rap = static_cast<STVectorF*>(fFragRapidity -> At(i)) -> fElements[itrack];
      if(rap > 0)
      {
        if(data -> recodpoca[itrack].Mag() < 20 && data -> vaNRowClusters[itrack] + data -> vaNLayerClusters[itrack] > 8 && prob > 0.2)
        {
          auto& P = static_cast<STVectorVec3*>(fCMVector -> At(i)) -> fElements[itrack];
          if(fSupportedPDG[i] == 2212 || fSupportedPDG[i] == 1000010020 || fSupportedPDG[i] == 1000010030 || fSupportedPDG[i] == 1000020030 || fSupportedPDG[i] == 1000020040)
          {
            N_H_He += prob;
            N_H_He_pt += (P.Perp()*prob);
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
  if(N_H_He > 0) N_H_He_pt /= N_H_He;
  if(N > 0) Npt /= N;

  fOutput << et << "\t" << erat << "\t" << Mch << "\t";
  fOutput << N_H_He << "\t" << N_H_He_pt << "\t" << N << "\t";
  fOutput << Npt << "\n";
}
