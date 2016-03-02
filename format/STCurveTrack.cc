/**
 * @brief SpiRITROOT curve track container
 *
 * @author JungWoo Lee
 */

#include "STCurveTrack.hh"

ClassImp(STCurveTrack)

STCurveTrack::STCurveTrack()
{
  Clear();
}

STCurveTrack::~STCurveTrack()
{
}

STCurveTrack::STCurveTrack(STCurveTrack *track)
{
  fTrackID   = track -> GetTrackID();
  fIsPrimary = track -> IsPrimary();
  fIsFitted  = track -> IsFitted();
  fIsSorted  = track -> IsSorted();

  fXVertex[0] = track -> GetXVertex(0);
  fYVertex[0] = track -> GetYVertex(0);
  fZVertex[0] = track -> GetZVertex(0);

  fXVertex[1] = track -> GetXVertex(1);
  fYVertex[1] = track -> GetYVertex(1);
  fZVertex[1] = track -> GetZVertex(1);

  fXDirection = track -> GetXDirection();
  fYDirection = track -> GetYDirection();
  fZDirection = track -> GetZDirection();

  fXNormal = track -> GetXNormal();
  fYNormal = track -> GetYNormal();
  fZNormal = track -> GetZNormal();

  fChargeSum = track -> GetChargeSum();
  fNumHits = track -> GetNumHits();

  fHitPtrArray = *(track -> GetHitPointerArray()); 
  fHitIDArray  = *(track -> GetHitIDArray()); 

  fClusterPtrArray = *(track -> GetClusterPointerArray()); 
  fClusterIDArray  = *(track -> GetClusterIDArray()); 

  fRMSLine  = track -> GetRMSLine();
  fRMSPlane = track -> GetRMSPlane();

  fExpectationX = track -> GetExpectationX();
  fExpectationY = track -> GetExpectationY();
  fExpectationZ = track -> GetExpectationZ();

  fExpectationXX = track -> GetExpectationXX();
  fExpectationYY = track -> GetExpectationYY();
  fExpectationZZ = track -> GetExpectationZZ();

  fExpectationXY = track -> GetExpectationXY();
  fExpectationYZ = track -> GetExpectationYZ();
  fExpectationZX = track -> GetExpectationZX();

  fNumHitsRemoved = GetNumHitsRemoved();
}

STCurveTrack::STCurveTrack(Int_t trackID, STHit* hit)
{
  Clear();
  fTrackID = trackID;
  AddHit(hit);
}

void STCurveTrack::Clear(Option_t *option)
{
  fTrackID   = -1;
  fIsPrimary = kFALSE;
  fIsFitted  = kFALSE;
  fIsSorted  = kFALSE;

  fXVertex[0] = 0;
  fYVertex[0] = 0;
  fZVertex[0] = 0;

  fXVertex[1] = 0;
  fYVertex[1] = 0;
  fZVertex[1] = 0;

  fXDirection = 0;
  fYDirection = 0;
  fZDirection = 0;

  fXNormal = 0;
  fYNormal = 0;
  fZNormal = 0;

  fChargeSum = 0;
  fNumHits = 0;
  fNumHitsRemoved = 0;

  fRMSLine = -1;
  fRMSPlane = -1;

  fExpectationX = 0;
  fExpectationY = 0;
  fExpectationZ = 0;

  fExpectationXX = 0;
  fExpectationYY = 0;
  fExpectationZZ = 0;

  fExpectationXY = 0;
  fExpectationYZ = 0;
  fExpectationZX = 0;

  fHitIDArray.clear();
  fClusterIDArray.clear();

  if (TString(option) == "C") {
    DeleteHits();
    DeleteClusters();
  }
  else {
    fHitPtrArray.clear();
    fClusterPtrArray.clear();
  }
}

void STCurveTrack::DeleteClusters()
{
  UInt_t size1 = fClusterPtrArray.size();
  for (UInt_t iCluster = 0; iCluster < size1; iCluster++)
    delete fClusterPtrArray[iCluster];

  fClusterPtrArray.clear();
}

void STCurveTrack::RemoveHit(STHit *hit)
{
  fIsFitted = kFALSE;

  TVector3 position = hit -> GetPosition();
  Double_t charge   = hit -> GetCharge();

  fExpectationX = (fChargeSum * fExpectationX - charge * position.X()) / (fChargeSum - charge);
  fExpectationY = (fChargeSum * fExpectationY - charge * position.Y()) / (fChargeSum - charge);
  fExpectationZ = (fChargeSum * fExpectationZ - charge * position.Z()) / (fChargeSum - charge);

  fExpectationXX = (fChargeSum * fExpectationXX - charge * position.X() * position.X()) / (fChargeSum - charge);
  fExpectationYY = (fChargeSum * fExpectationYY - charge * position.Y() * position.Y()) / (fChargeSum - charge);
  fExpectationZZ = (fChargeSum * fExpectationZZ - charge * position.Z() * position.Z()) / (fChargeSum - charge);

  fExpectationXY = (fChargeSum * fExpectationXY - charge * position.X() * position.Y()) / (fChargeSum - charge);
  fExpectationYZ = (fChargeSum * fExpectationYZ - charge * position.Y() * position.Z()) / (fChargeSum - charge);
  fExpectationZX = (fChargeSum * fExpectationZX - charge * position.Z() * position.X()) / (fChargeSum - charge);

  fChargeSum -= charge;

  fNumHitsRemoved++;
}

Int_t STCurveTrack::GetNumHitsRemoved() const { return fNumHitsRemoved; }

void STCurveTrack::AddCluster(STHitCluster *cluster)
{
  fClusterIDArray.push_back(cluster -> GetClusterID());
  fClusterPtrArray.push_back(cluster);
}

Int_t STCurveTrack::GetNumClusters() { return fClusterIDArray.size(); }

std::vector<Int_t> 
*STCurveTrack::GetClusterIDArray() { return &fClusterIDArray; }

std::vector<STHitCluster*> 
*STCurveTrack::GetClusterPointerArray() { return &fClusterPtrArray; }

Int_t  STCurveTrack::GetClusterID(Int_t i)      { return fClusterIDArray.at(i); };
STHitCluster *STCurveTrack::GetCluster(Int_t i) { return fClusterPtrArray.at(i); }
