#include "STExampleAnaTask.hh"

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
#include "TF1.h"
#include "TH2.h"
#include "TPad.h"

#include <cmath>

ClassImp(STExampleAnaTask);

STExampleAnaTask::STExampleAnaTask()
{
  fLogger = FairLogger::GetLogger(); 
}

STExampleAnaTask::~STExampleAnaTask()
{}

double STExampleAnaTask::Correction(const TVector2& Q_vec)
{
  double phi = -9999;
  if(Q_vec.Mod() > 0)
  {
    if(fShift && fQx_mean && fQy_mean) 
    {
      phi = this -> Shifting(Q_vec);
      if(fFlat && fAn && fBn) phi = this -> Flattening(phi);
    }
    else
      phi = Q_vec.Phi();
    phi = (phi > TMath::Pi())? phi - 2*TMath::Pi() : phi;
  }
  else phi = -9999;
  return phi;
}

double STExampleAnaTask::Shifting(TVector2 Q_vec)
{
  Q_vec.SetX((Q_vec.X() - fQx_mean -> GetVal())/fQx_sigma -> GetVal());
  Q_vec.SetY((Q_vec.Y() - fQy_mean -> GetVal())/fQy_sigma -> GetVal());
  return  Q_vec.Phi();
}

double STExampleAnaTask::Flattening(double phi)
{
  double delta_phi = 0;
  for(int i = 0; i < fAn -> fElements.size(); ++i)
  {
    int n = i + 1;
    delta_phi += fAn -> fElements[i]*cos(n*phi) + fBn -> fElements[i]*sin(n*phi);
  }
  return phi + delta_phi;
}

InitStatus STExampleAnaTask::Init()
{
  STAnaParticleDB::FillTDatabasePDG();
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  return kSUCCESS;
}

void STExampleAnaTask::SetParContainers()
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

void STExampleAnaTask::Exec(Option_t *opt)
{
  for(int i = 0; i < fSupportedPDG.size(); ++i)
  {
    int pdg = fSupportedPDG[i];
    // do what you need to do with that particle
    // remember your TClonesArray should align with fSupportedPDG
  }
}

void STExampleAnaTask::FinishTask()
{
  auto outfile = FairRootManager::Instance() -> GetOutFile();
  outfile -> cd();
  // write any meta data to output
}


