// =================================================
//  STHitCluster Class                          
//                                                  
//  Description:                                    
//    Container for a cluster of hits
//                                                  
//  Genie Jhang ( geniejhang@majimak.com )          
//  2014. 07. 15                                    
// =================================================

#include "STHitCluster.hh"

ClassImp(STHitCluster)

STHitCluster::STHitCluster()
{
  fClusterID = -1;

  fPosition = TVector3(0, 0, -1000);
  fPosSigma = TVector3(0., 0., 0.);

  fCovariant.ResizeTo(3, 3);
  for (Int_t iElem = 0; iElem < 9; iElem++)
    fCovariant(iElem/3, iElem%3) = 0;
}

STHitCluster::STHitCluster(STHitCluster *cluster)
{
  fClusterID = cluster -> GetClusterID();

  fPosition = cluster -> GetPosition();
  fPosSigma = cluster -> GetPosSigma();

  fCovariant.ResizeTo(3, 3);
  fCovariant = cluster -> GetCovMatrix();

  fHitIDArray = *(cluster -> GetHitIDs());
}

STHitCluster::~STHitCluster()
{
}

void STHitCluster::SetClusterID(Int_t clusterID) { fClusterID = clusterID; }
Int_t STHitCluster::GetClusterID()       { return fClusterID; }

TVector3 STHitCluster::GetPosition()     { return fPosition; }
TVector3 STHitCluster::GetPosSigma()     { return fPosSigma; }
TMatrixD STHitCluster::GetCovMatrix()    { return fCovariant; }

Int_t STHitCluster::GetNumHits()         { return fHitIDArray.size(); }
vector<Int_t> *STHitCluster::GetHitIDs() { return &fHitIDArray; }

void STHitCluster::AddHit(STHit *hit)
{
  fHitIDArray.push_back(hit -> GetHitID());
  hit -> SetClusterID(fClusterID);

  // Calculating cluster position, error and covariant matrix
}
