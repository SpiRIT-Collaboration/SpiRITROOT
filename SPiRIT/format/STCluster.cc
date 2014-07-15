// =================================================
//  STCluster Class                          
//                                                  
//  Description:                                    
//    Container for a cluster data
//                                                  
//  Genie Jhang ( geniejhang@majimak.com )          
//  2014. 07. 15                                    
// =================================================

#include "STCluster.hh"

ClassImp(STCluster)

STCluster::STCluster()
{
  fPosition = TVector3(0, 0, -1000);
  fPosSigma = TVector3(0., 0., 0.);

  fCovariant = TMatrix(3, 3);
  for (Int_t iElem = 0; iElem < 9; iElem++)
    fCovariant(iElem/3, iElem%3) = 0;
}

STCluster::STCluster(STCluster *cluster)
{
  fPosition = cluster -> GetPosition();
  fPosSigma = cluster -> GetPosSigma();

  fCovariant = TMatrix(3, 3);
  fCovariant = cluster -> GetCovMatrix();

  fHitNoArray = *(cluster -> GetHitNumbers());
}

STCluster::~STCluster()
{
}

TVector3 STCluster::GetPosition()  { return fPosition; }
TVector3 STCluster::GetPosSigma()  { return fPosSigma; }
TMatrixD STCluster::GetCovMatrix() { return fCovariant; }

Int_t STCluster::GetNumHits()                  { return fHitNoArray.size(); }
std::vector<Int_t> *STCluster::GetHitNumbers() { return &fHitNoArray; }

void STCluster::AddHit(STHit *hit)
{
  // Calculating cluster position, error and covariant matrix
}
