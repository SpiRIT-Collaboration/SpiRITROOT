// =================================================
//  STCluster Class                          
//                                                  
//  Description:                                    
//    Container for a cluster data
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

class STCluster : public TObject 
{
  public:
    STCluster();
    STCluster(STCluster *cluster);
    ~STCluster();

    TVector3 GetPosition();
    TVector3 GetPosError();
    TMatrixD GetCovMatrix();

    Int_t GetNumHits();
    std::vector<Int_t> *GetHitNumbers();

    void AddHit(STHit *hit);

  private:
    TVector3 fPosition;
    TVector3 fPosError;
    TMatrixD fCovariant;

    std::vector<Int_t> fHitNoArray;

  ClassDef(STCluster, 1)
};

#endif
