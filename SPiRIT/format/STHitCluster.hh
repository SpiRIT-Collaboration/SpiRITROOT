// =================================================
//  STHitCluster Class                          
//                                                  
//  Description:                                    
//    Container for a cluster of hits
//                                                  
//  Genie Jhang ( geniejhang@majimak.com )          
//  2014. 07. 15                                    
// =================================================

#ifndef STCLUSTER_H
#define STCLUSTER_H

#include "STHit.hh"

#include "TObject.h"
#include "TVector3.h"
#include "TMatrixD.h"

#include <vector>

class STHitCluster : public TObject 
{
  public:
    STHitCluster();
    STHitCluster(STHitCluster *cluster);
    ~STHitCluster();

    TVector3 GetPosition();
    TVector3 GetPosSigma();
    TMatrixD GetCovMatrix();

    Int_t GetNumHits();
    std::vector<Int_t> *GetHitNumbers();

    void AddHit(STHit *hit);

  private:
    TVector3 fPosition;
    TVector3 fPosSigma;
    TMatrixD fCovariant;

    std::vector<Int_t> fHitNoArray;

  ClassDef(STHitCluster, 1)
};

#endif
