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
  fPosition = TVector3(0, 0, -1000);
  fPosSigma = TVector3(0., 0., 0.);

  fCovariant.ResizeTo(3, 3);
  for (Int_t iElem = 0; iElem < 9; iElem++)
    fCovariant(iElem/3, iElem%3) = 0;
}

STHitCluster::STHitCluster(STHitCluster *cluster)
{
  fPosition = cluster -> GetPosition();
  fPosSigma = cluster -> GetPosSigma();

  fCovariant.ResizeTo(3, 3);
  fCovariant = cluster -> GetCovMatrix();

  fHitNoArray = *(cluster -> GetHitNumbers());
}

STHitCluster::~STHitCluster()
{
}

TVector3 STHitCluster::GetPosition()  { return fPosition; }
TVector3 STHitCluster::GetPosSigma()  { return fPosSigma; }
TMatrixD STHitCluster::GetCovMatrix() { return fCovariant; }

Int_t STHitCluster::GetNumHits()                  { return fHitNoArray.size(); }
std::vector<Int_t> *STHitCluster::GetHitNumbers() { return &fHitNoArray; }

void STHitCluster::AddHit(STHit *hit)
{
  // Calculating cluster position, error and covariant matrix
}
