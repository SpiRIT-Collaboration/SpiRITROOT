// SpiRITROOT classes
#include "STClusterizer.hh"
#include "STParReader.hh"

// FairRoot classes
#include "FairRuntimeDb.h"
#include "FairRun.h"

// STL
#include <algorithm>

STClusterizer::STClusterizer()
{
  fLogger = FairLogger::GetLogger();

  FairRun *run = FairRun::Instance();

  if (run)
  {
    FairRuntimeDb *db = run -> GetRuntimeDb();
    if (!db)
      fLogger -> Fatal(MESSAGE_ORIGIN, "No runtime database!");

    STDigiPar* par = (STDigiPar *) db -> getContainer("STDigiPar");
    if (!par)
      fLogger -> Fatal(MESSAGE_ORIGIN, "STDigiPar not found!!");

    fPrimaryVertex = TVector3(0., -213.3, -35.2);

    fPadPlaneX      = par -> GetPadPlaneX();
    fPadSizeX       = par -> GetPadSizeX();
    fPadSizeZ       = par -> GetPadSizeZ();
    fPadRows        = par -> GetPadRows();
    fPadLayers      = par -> GetPadLayers();
    fNumTbs         = par -> GetWindowNumTbs();
    fTBTime         = par -> GetTBTime();
    fDriftVelocity  = par -> GetDriftVelocity();
    fMaxDriftLength = par -> GetDriftLength();

    TString parName = par -> GetTrackingParFileName();
    STParReader *trackingPar = new STParReader(parName);
    fNumHitsAtHead = trackingPar -> GetIntPar("NumHitsAtHead");
  }

  else
  {
    TString workDir = gSystem -> Getenv("VMCWORKDIR");
    TString parameterFile = workDir + "/parameters/ST.parameters.par";
    STParReader *fParReader = new STParReader(parameterFile);

    fPadPlaneX      = fParReader -> GetDoublePar("PadPlaneX");
    fPadSizeX       = fParReader -> GetDoublePar("PadSizeX");
    fPadSizeZ       = fParReader -> GetDoublePar("PadSizeZ");
    fPadRows        = fParReader -> GetIntPar("PadRows");
    fPadLayers      = fParReader -> GetIntPar("PadLayers");
    fNumTbs         = fParReader -> GetIntPar("NumTbs");
    fDriftVelocity  = fParReader -> GetDoublePar("DriftVelocity");
    fMaxDriftLength = fParReader -> GetDoublePar("DriftLength");

    Int_t samplingRate = fParReader -> GetIntPar("SamplingRate");
    switch (samplingRate) {
      case 25:  fTBTime = 40; break;
      case 50:  fTBTime = 20; break;
      case 100: fTBTime = 10; break;
      default:  fTBTime = -1; break;
    }

    TString parName = workDir + "/parameters/CurveTracking.par";
    STParReader *trackingPar = new STParReader(parName);
    fNumHitsAtHead = trackingPar -> GetIntPar("NumHitsAtHead");
  }

  fYTb = fTBTime * fDriftVelocity/100.;

  fXLowCut  = -424.;
  fXHighCut =  424.;

  SetProximityCutInUnit(1.5, 2.5, 0.5);
  SetSigmaCutInUnit(2.0, 2.0, 0.8);
}

STClusterizer::~STClusterizer()
{
}

void STClusterizer::SetPrimaryVertex(TVector3 vertex) { fPrimaryVertex = vertex; }
void STClusterizer::SetProximityCut(Double_t x, Double_t y, Double_t z)
{
  fXCut = x;
  fYCut = y;
  fZCut = z;
}
void STClusterizer::SetProximityCutInUnit(Double_t xw, Double_t yw, Double_t zw)
{
  fXCut = fPadSizeX * xw;
  fYCut = fYTb * yw;
  fZCut = fPadSizeZ * zw;
}
void STClusterizer::SetSigmaCut(Double_t x, Double_t y, Double_t z)
{
  fSigmaXCut = x;
  fSigmaYCut = y;
  fSigmaZCut = z;
}
void STClusterizer::SetSigmaCutInUnit(Double_t xw, Double_t yw, Double_t zw)
{
  fSigmaXCut = fPadSizeX * xw;
  fSigmaYCut = fYTb * yw;
  fSigmaZCut = fPadSizeZ * zw;
}
void STClusterizer::SetEdgeCut(Double_t low, Double_t high)
{
  fXLowCut = low;
  fXHighCut = high;
}

void STClusterizer::SetClusteringOption(Int_t opt) { fClusteringOption = opt; }
