/**
 *  @brief STHit Class
 *
 *  @author Genie Jhang ( geniejhang@majimak.com )
 *  @author JungWoo Lee
 */

#ifndef STHITCLUSTER_HH
#define STHITCLUSTER_HH

#include "STHit.hh"

#include "TObject.h"
#include "TVector3.h"
#include "TMatrixD.h"

#include <vector>

using std::vector;

class STHitCluster : public STHit
{
  public:
    STHitCluster();
    STHitCluster(STHitCluster *cluster);
    virtual ~STHitCluster() {}

    void SetCovMatrix(TMatrixD matrix);  ///< Set covariance matrix

    Bool_t IsClustered() const;
     Int_t GetHitID() const;

         TMatrixD   GetCovMatrix() const; ///< Get covariance matrix
            Int_t   GetNumHits();         ///< Get number of hits
    vector<Int_t>  *GetHitIDs();          ///< Get vector array hit IDs
    vector<STHit*> *GetHitPtrs();

    virtual void AddHit(STHit *hit);      ///< Add hit to cluster

    virtual void SetClusterID(Int_t clusterID);

    void SetLength(Double_t length);
    Double_t GetLength();

    void SetIsStable(Bool_t isStable = true);
    Bool_t IsStable();

    void SetPOCA(TVector3 p);
    TVector3 GetPOCA();

  protected:
    TMatrixD fCovMatrix;                  ///< Cluster covariance matrix
    vector<Int_t>  fHitIDArray;           ///< Vector array of hit IDs
    vector<STHit*> fHitPtrArray;          //! <

    Double_t fLength;

    Double_t fPOCAX;
    Double_t fPOCAY;
    Double_t fPOCAZ;

    /**
     * Calculate weighted mean for cluster position. (Weight = charge) <br>
     * Weighted mean: \f$ \mu_{n+1} = \mu_n + \displaystyle\frac{a_{n+1} - \mu_n}{W_n+w_{n+1}},\quad(n\geq0). \f$
     */
    void CalculatePosition(TVector3 hitPos, Double_t charge);

    /**
     * Calculate weighted covariance matrix for each variable. (Weight = charge) <br>
     * Cluster position uncertainty is also calculated here by taking the square root of diagonal components. <br>
     * Weighted covariance matrix: \f$\sigma(a,b)_{n+1}=\displaystyle\frac{W_n}{W_n+w_{n+1}}\sigma(a,b)_{n}+\displaystyle\frac{(\mu_{n+1}-a_{n+1})(\nu_{n+1}-b_{n+1})}{W_n},\quad(n\geq1).\f$
     */
    void CalculateCovMatrix(TVector3 hitPos, Double_t charge); 

  ClassDef(STHitCluster, 7);
};

#endif
