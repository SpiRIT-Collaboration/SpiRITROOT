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

using std::vector;

class STHitCluster : public TObject 
{
  public:
    STHitCluster();
    STHitCluster(STHitCluster *cluster);
    ~STHitCluster();

    void SetClusterID(Int_t clusterID);
    Int_t GetClusterID();

    void SetPosition(TVector3 vector);
    void SetPosSigma(TVector3 vector);
    void SetCovMatrix(TMatrixD matrix);
    void SetCharge(Double_t charge);

    TVector3 GetPosition();
    TVector3 GetPosSigma();
    TMatrixD GetCovMatrix();
    Double_t GetCharge();

    Int_t GetNumHits();
    vector<Int_t> *GetHitIDs();

    void AddHit(STHit *hit);

  private:
    Int_t fClusterID;     //!< Cluster ID
    TVector3 fPosition;   //!< Cluster position
    TVector3 fPosSigma;   //!< Cluster position uncertainty
    TMatrixD fCovMatrix;  //!< Cluster covariance matrix
    Double_t fCharge;     //!< Cluster Charge

    vector<Int_t> fHitIDArray;

    void CalculatePosition(TVector3 hitPos, Double_t charge);
    void CalculateCovMatrix(TVector3 hitPos, Double_t charge);

  ClassDef(STHitCluster, 1)
};

#endif
