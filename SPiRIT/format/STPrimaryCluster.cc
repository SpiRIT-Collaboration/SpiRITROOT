//---------------------------------------------------------------------
// Description:
//      Primary Cluster data class source
//
//      Data class for primary clusters for STClusterizerTask.
//      Primary cluster is cluster of electrons produced from 
//      STMCPoint.
//
//      Created by : STClusterizerTask
//      Used    by : STDriftTask
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

// This class header
#include "STPrimaryCluster.hh"

ClassImp(STPrimaryCluster);

STPrimaryCluster::STPrimaryCluster()
: fCharge(0),
  fPosition(TVector3(0,0,0)),
  fTime(0),
  fTrackID(0),
  fHitID(0)
{
}

STPrimaryCluster::STPrimaryCluster(UInt_t   charge,
                                   TVector3 position,
                                   Double_t time,
                                   UInt_t   trackID,
                                   Int_t    hitID)
: fCharge(charge),
  fPosition(position),
  fTime(time),
  fTrackID(trackID),
  fHitID(hitID)
{
}

STPrimaryCluster::~STPrimaryCluster()
{
}

// Getters
UInt_t   STPrimaryCluster::GetCharge()    { return fCharge; }
TVector3 STPrimaryCluster::GetPosition()  { return fPosition; }
Double_t STPrimaryCluster::GetPositionX() { return fPosition(0); }
Double_t STPrimaryCluster::GetPositionY() { return fPosition(1); }
Double_t STPrimaryCluster::GetPositionZ() { return fPosition(2); }
Double_t STPrimaryCluster::GetTime()      { return fTime; }
UInt_t   STPrimaryCluster::GetTrackID()   { return fTrackID; }
Int_t    STPrimaryCluster::GetHitID()     { return fHitID; }

// Setters
void     STPrimaryCluster::SetIndex(Int_t index) { fIndex = index; }
