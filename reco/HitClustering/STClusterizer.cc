// SpiRITROOT classes
#include "STClusterizer.hh"

// FairRoot classes
#include "FairRuntimeDb.h"
#include "FairRun.h"

// STL
#include <algorithm>

STClusterizer::STClusterizer()
{
  fLogger = FairLogger::GetLogger();

  FairRun *run = FairRun::Instance();
  if (!run)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No analysis run!");

  FairRuntimeDb *db = run -> GetRuntimeDb();
  if (!db)
    fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

  fPar = (STDigiPar *) db -> getContainer("STDigiPar");
  if (!fPar)
    fLogger -> Fatal(MESSAGE_ORIGIN, "STDigiPar not found!!");

  fPadPlaneX = fPar -> GetPadPlaneX();
  fPadSizeX = fPar -> GetPadSizeX();
  fPadSizeZ = fPar -> GetPadSizeZ();
  fPadRows = fPar -> GetPadRows();
  fPadLayers = fPar -> GetPadLayers();

  fNumTbs = fPar -> GetWindowNumTbs();
  fTBTime = fPar -> GetTBTime();
  fDriftVelocity = fPar -> GetDriftVelocity();
  fMaxDriftLength = fPar -> GetDriftLength();

  fYTb = fTBTime*fDriftVelocity/100.;
}

void STClusterizer::SetProximityCut(Double_t x, Double_t y, Double_t z)
{
  fXCut = x;
  fYCut = y;
  fZCut = z;
}
void STClusterizer::SetSigmaCut(Double_t x, Double_t y, Double_t z)
{
  fSigmaXCut = x;
  fSigmaYCut = y;
  fSigmaZCut = z;
}

STClusterizer::~STClusterizer()
{
}
