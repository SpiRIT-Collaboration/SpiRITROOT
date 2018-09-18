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

    void Clear(Option_t * = "");

    void SetCovMatrix(TMatrixD matrix);  ///< Set covariance matrix

    Bool_t IsEmbed() const;
    Bool_t IsClustered() const;
    Bool_t IsMissingCharge() const;
     Int_t GetHitID() const;

    TMatrixD   GetCovMatrix() const;          ///< Get covariance matrix
    Int_t   GetNumHits()  const;              ///< Get number of hits
    Int_t   GetNumSatNeighbors()  const;     ///< Get # of saturated pads neighbored hits in cluster
    Double_t   GetFracSatNeighbors()  const; ///< Get fract of saturated pads neighbored hits in cluster
    vector<Int_t>  *GetHitIDs();          ///< Get vector array hit IDs
    vector<STHit*> *GetHitPtrs();

    virtual void AddHit(STHit *hit);      ///< Add hit to cluster
    void ApplyModifiedHitInfo();          ///< Apply changes that was made to hit position and charge

    void SetDFromCovForGenfit(Double_t maxx, Double_t maxy, Double_t maxz, bool setMin = false);

    virtual void SetClusterID(Int_t clusterID);

    void SetIsMissingCharge(Bool_t val); //True if a hit neighbors a dead pad therefore missing charge
    void SetFractSatNeighbors(Double_t frac);  //fraction of hits which have neighboring dead pads
    void SetNumSatNeighbors(Int_t num);        //# of hits which have neighboring dead pads
    void SetLength(Double_t length);
    Double_t GetLength();

    void SetIsEmbed(Bool_t);
    void SetIsStable(Bool_t isStable = true);
    Bool_t IsStable();

    void SetPOCA(TVector3 p);
    TVector3 GetPOCA();

    bool IsLayerCluster() const { return (fRow == -1 && fLayer != -1) ? true : false; }
    bool IsRowCluster()   const { return (fRow != -1 && fLayer == -1) ? true : false; }

    void SetIsContinuousHits(Bool_t value = kTRUE) { fIsContinuousHits = value; };
    Bool_t IsContinuousHits() const { return fIsContinuousHits; }

    void ApplyCovLowLimit();

    void SetChi(Double_t chi) { fChi = chi; } 
    Double_t GetChi() const { return fChi; }

    void SetLambda(Double_t dip) { fLambda = dip; }
    Double_t GetLambda() const { return fLambda; }

  protected:
    TMatrixD fCovMatrix;                  ///< Cluster covariance matrix
    vector<Int_t>  fHitIDArray;           ///< Vector array of hit IDs
    vector<STHit*> fHitPtrArray;          //! <

    Int_t fnumSatNeigh;                  // number of hits neighboring saturated pads
    Double_t fLength;

    Double_t fPOCAX;
    Double_t fPOCAY;
    Double_t fPOCAZ;
    Double_t fFractionSat;              //Fraction of hits in cluster with next to dead pad
    Bool_t fIsEmbed;
    Bool_t fIsMissingCharge;            //Missing charge comes from hits neighboring dead pads

    Double_t fChi; // polar angle in pad plane from z-axis
    Double_t fLambda; // dip angle measured from pad plane


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

    Bool_t fIsContinuousHits;

  ClassDef(STHitCluster, 10);
};

#endif
