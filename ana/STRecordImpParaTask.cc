#include "STRecordImpParaTask.hh"

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

ClassImp(STRecordImpParaTask);

STRecordImpParaTask::STRecordImpParaTask()
{
  fLogger = FairLogger::GetLogger(); 
}

STRecordImpParaTask::~STRecordImpParaTask()
{}

InitStatus STRecordImpParaTask::Init()
{
  STAnaParticleDB::FillTDatabasePDG();
  FairRootManager *ioMan = FairRootManager::Instance();
  if (ioMan == 0) {
    fLogger -> Error(MESSAGE_ORIGIN, "Cannot find RootManager!");
    return kERROR;
  }

  fSkip = (STVectorI*) ioMan -> GetObject("Skip");
  fMLImp = (STVectorF*) ioMan -> GetObject("bML");
  fMultImp = (STVectorF*) ioMan -> GetObject("bMult");
  fTrueImp = (STVectorF*) ioMan -> GetObject("bTruth");

  if(fMLImp) 
  {
    fLogger -> Info(MESSAGE_ORIGIN, "Found bML branch. Will plot impact parameter distribution from ML.");
    fMLHist = new TH1F("MLHist", "MLHist", 20, 0, 10);
  }
  if(fMultImp) 
  {
    fLogger -> Info(MESSAGE_ORIGIN, "Found bMult branch. Will plot impact parameter distribution from Multiplicity.");
    fMultHist = new TH1F("MultHist", "MultHist", 20, 0, 10);
  }
  if(fTrueImp) 
  {
    fLogger -> Info(MESSAGE_ORIGIN, "Found bTruth branch. Will plot true impact parameter distribution.");
    fTrueHist = new TH1F("TrueHist", "TrueHist", 20, 0, 10);
  }
  if(fMultImp && fMLImp) fMLVsMult = new TH2F("MLVsMult", ";Mult;ML", 50, 0, 10, 50, 0, 10);

  return kSUCCESS;
}

void STRecordImpParaTask::SetParContainers()
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

void STRecordImpParaTask::Exec(Option_t *opt)
{
  if(fSkip)
    if(fSkip -> fElements[0] == 1) return; // skip == 1 indicates event skip

  if(fMLHist) fMLHist -> Fill(fMLImp -> fElements[0]);
  if(fMultHist) 
  {
    double bhat = fMultImp -> fElements[0];
    double b = bhat*fBMax + fSmear*gRandom->Uniform(-0.5, 0.5)*(bhat - 1);
    fMultHist -> Fill(b);
    if(fMLVsMult)
      fMLVsMult -> Fill(b, fMLImp -> fElements[0]);

  }
  if(fTrueHist) fTrueHist -> Fill(fTrueImp -> fElements[0]);
}

void STRecordImpParaTask::FinishTask()
{
  auto outfile = FairRootManager::Instance() -> GetOutFile();
  outfile -> cd();
  // write any meta data to output
   if(fMLHist) fMLHist -> Write();
   if(fMultHist) fMultHist -> Write();
   if(fTrueHist) fTrueHist -> Write();
   if(fMLVsMult) fMLVsMult -> Write();
}


