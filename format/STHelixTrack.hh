#ifndef STHELIXTRACK
#define STHELIXTRACK

#include "STHit.hh"
#include "STHitCluster.hh"
#include <vector>

/**
 * SpiRIT Helix Track Container.
 *
 * All units in [mm], [ADC], [radian], [MeV].
 */

class STHelixTrack : public TObject
{
  public:
    STHelixTrack();
    STHelixTrack(Int_t id);

    enum STFitStatus { kBad, kLine, kPlane, kHelix, kGenfitTrack };

  private:
    Int_t fTrackID;
    Int_t fParentID;

    STFitStatus fFitStatus;  ///< One of kBad, kHelix and kLine.

    Bool_t fIsEmbed;         ///If embedded cluster exists then helix is embedded
    Double_t fXHelixCenter;  ///< x-component of the helix center
    Double_t fZHelixCenter;  ///< z-component of the helix center
    Double_t fHelixRadius;   ///< Radius of the helix
    Double_t fYInitial;      ///< y-position at angle alpha = 0
    Double_t fAlphaSlope;    ///< y = fAlphaSlope * alpha + fYInitial

    Double_t fChargeSum;     ///< Sum of charge

    Double_t fExpectationX;  //! < Expectation value of x
    Double_t fExpectationY;  //! < Expectation value of y
    Double_t fExpectationZ;  //! < Expectation value of z
    Double_t fExpectationXX; //! < Expectation value of x^2
    Double_t fExpectationYY; //! < Expectation value of y^2
    Double_t fExpectationZZ; //! < Expectation value of z^2
    Double_t fExpectationXY; //! < Expectation value of xy
    Double_t fExpectationYZ; //! < Expectation value of yz
    Double_t fExpectationZX; //! < Expectation value of zx

    Double_t fRMSW;          ///< width RMS of the fit
    Double_t fRMSH;          ///< height RMS of the fit

    Double_t fAlphaHead;     ///< Head position alpha
    Double_t fAlphaTail;     ///< Last position alpha

    Bool_t fIsPositiveChargeParticle;
    TVector3 fVertexPosition; //! < Temporary vertex position for sorting

    std::vector<STHit *> fMainHits; //!
    std::vector<STHit *> fCandHits; //!
    std::vector<STHitCluster *> fHitClusters; //!

    std::vector<Int_t> fMainHitIDs;    ///<
    std::vector<Int_t> fClusterIDs;    ///<
    std::vector<Double_t> fdEdxArray;  ///< dE/dx array;

    Int_t    fGenfitID;        ///< GENFIT Track ID
    Double_t fGenfitMomentum;  ///< Momentum reconstructed by GENFIT

  public:
    void Clear(Option_t *option = "");
    virtual void Print(Option_t *option="") const;

    void AddHit(STHit *hit);
    void Remove(STHit *hit);
    void DeleteHits();
    void SortHits(bool increasing = true);
    void SortClusters(bool increasing = true);
    void SortHitsByTimeOrder();
    void SortClustersByTimeOrder();

    void AddHitCluster(STHitCluster *cluster);
    void AddHitClusterAtFront(STHitCluster *cluster);

    void FinalizeHits();
    void FinalizeClusters();

    void SetTrackID(Int_t idx);
    void SetGenfitID(Int_t idx);
    void SetParentID(Int_t idx);

    void SetFitStatus(STFitStatus value);
    void SetIsEmbed(Bool_t val);
    void SetIsBad();
    void SetIsLine();
    void SetIsPlane();
    void SetIsHelix();
    void SetIsGenfitTrack();

    void SetLineDirection(TVector3 dir);  ///< ONLY USED IN TRACK FINDING
    void SetPlaneNormal(TVector3 norm);   ///< ONLY USED IN TRACK FINDING

    void SetHelixCenter(Double_t x, Double_t z);
    void SetHelixRadius(Double_t r);
    void SetYInitial(Double_t y);
    void SetAlphaSlope(Double_t s);

    void SetRMSW(Double_t rms);
    void SetRMSH(Double_t rms);
    void SetAlphaHead(Double_t alpha);
    void SetAlphaTail(Double_t alpha);

    void DetermineParticleCharge(TVector3 vertex);
    void SetIsPositiveChargeParticle(Bool_t val);

    void SetGenfitMomentum(Double_t p);

    Int_t GetTrackID() const;
    Int_t GetGenfitID() const;
    Int_t GetParentID() const;

    STFitStatus GetFitStatus() const;
    TString GetFitStatusString() const;
    bool IsEmbed() const;
    bool IsBad() const;
    bool IsLine() const;
    bool IsPlane() const;
    bool IsHelix() const;
    bool IsGenfitTrack() const;

    Double_t GetHelixCenterX() const;
    Double_t GetHelixCenterZ() const;
    Double_t GetHelixRadius() const;
    Double_t GetYInitial() const;
    Double_t GetAlphaSlope() const;

    void GetHelixParameters(Double_t &xCenter, 
        Double_t &zCenter, 
        Double_t &radius, 
        Double_t &dipAngle,
        Double_t &yInitial,
        Double_t &alphaSlope) const;

    Double_t GetChargeSum() const;

    TVector3 GetMean() const;
    Double_t GetXMean() const;
    Double_t GetYMean() const;
    Double_t GetZMean() const;
    Double_t GetXCov() const;
    Double_t GetZCov() const;

    Double_t CovWXX() const; ///< SUM_i {(x_centroid-x_i)*(x_centroid-x_i) }
    Double_t CovWYY() const; ///< SUM_i {(y_centroid-y_i)*(y_centroid-y_i) }
    Double_t CovWZZ() const; ///< SUM_i {(z_centroid-z_i)*(z_centroid-z_i) }

    Double_t CovWXY() const; ///< SUM_i {(x_centroid-x_i)*(y_centroid-y_i) }
    Double_t CovWYZ() const; ///< SUM_i {(y_centroid-y_i)*(z_centroid-z_i) }
    Double_t CovWZX() const; ///< SUM_i {(z_centroid-z_i)*(x_centroid-x_i) }

    Double_t GetExpectationX()  const;
    Double_t GetExpectationY()  const;
    Double_t GetExpectationZ()  const;

    Double_t GetExpectationXX() const;
    Double_t GetExpectationYY() const;
    Double_t GetExpectationZZ() const;

    Double_t GetExpectationXY() const;
    Double_t GetExpectationYZ() const;
    Double_t GetExpectationZX() const;

    Double_t GetRMSW() const;
    Double_t GetRMSH() const;
    Double_t GetAlphaHead() const;
    Double_t GetAlphaTail() const;

    Bool_t IsPositiveChargeParticle() const;



    Int_t GetNumHits() const;
    STHit *GetHit(Int_t idx) const;
    std::vector<STHit *> *GetHitArray();

    Int_t GetNumCandHits() const;
    std::vector<STHit *> *GetCandHitArray();

    Int_t GetNumClusters() const;
    Int_t GetNumEmbedClusters() const;
    Int_t GetNumStableClusters() const;
    STHitCluster *GetCluster(Int_t idx) const;
    std::vector<STHitCluster *> *GetClusterArray();



    Int_t GetNumHitIDs() const;
    Int_t GetHitID(Int_t idx) const;
    std::vector<Int_t> *GetHitIDArray();

    Int_t GetNumClusterIDs() const;
    Int_t GetClusterID(Int_t idx) const;
    std::vector<Int_t> *GetClusterIDArray();

    std::vector<Double_t> *GetdEdxArray();
    Double_t GetdEdxWithCut(Double_t lowR, Double_t highR) const;



    TVector3 GetLineDirection() const;     ///< ONLY USED IN TRACK FINDING
    TVector3 GetPlaneNormal() const;       ///< ONLY USED IN TRACK FINDING

    TVector3 PerpLine(TVector3 p) const;   ///< ONLY USED IN TRACK FINDING
    TVector3 PerpPlane(TVector3 p) const;  ///< ONLY USED IN TRACK FINDING

    Double_t GetGenfitMomentum() const;    /// Momentum reconstructed by genfit (if is set)
    /**
     * Angle between p and pt. Value becomes 0 on xz plane.
     * = tan(-fAlphaSlope/fHelixRadius) + pi/2
    */
    Double_t DipAngle() const;

    /** [Distance from point to center of helix in xz-plane] - [Helix radius] */
    Double_t DistCircle(TVector3 pointGiven) const;

       Int_t Charge() const;                        ///< Assumed charge of the track
       Int_t Helicity() const;                      ///< Helicity of track +/-
    TVector3 PositionAtHead() const;                ///< Position at head of helix
    TVector3 PositionAtTail() const;                ///< Position at tail of helix
    Double_t Momentum(Double_t B = 0.5) const;      ///< Momentum of track (B = 0.5 by default)
    Double_t TrackLength() const;                   ///< Length of track calculated from head to tail.
    Double_t LengthInPeriod() const;                ///< Length of track in one period
    Double_t YLengthInPeriod() const;               ///< y-length of track in one period
    Double_t LengthByAlpha(Double_t alpha) const;   ///< Length of track in change of alpha
    Double_t AlphaByLength(Double_t length) const;  ///< Angle alpha in change of length
    TVector3 PositionByAlpha(Double_t alpha) const; ///< Position at angle alpha [mm]
    TVector3 Direction(Double_t alpha) const;       ///< Direction at angle alpha

    /**
     * Extrapolate track due to alpha angle of the given point.
     * Returns alpha;
     */
    Double_t AlphaAtPosition(TVector3 p);

    /**
     * Extrapolate track due to given alpha.
     * Returns extrapolation length from the initial track reference position.
    */
    Double_t ExtrapolateToAlpha(Double_t alpha) const;

    /**
     * Extrapolate track due to given alpha.
     * Returns extrapolation length from the initial track reference position.
     *
     * @param alpha         given alpha.
     * @param pointOnHelix  extrapolated position on the helix.
    */
    Double_t ExtrapolateToAlpha(Double_t alpha, TVector3 &pointOnHelix) const;

    /**
     * Extrapolate track due to alpha angle of the given point.
     * Returns extrapolation length from the initial track reference position.
     *
     * @param pointGiven    given point.
     * @param pointOnHelix  extrapolated position on the helix.
     * @param alpha         extrapolated alpha angle.
    */
    Double_t ExtrapolateToPointAlpha(TVector3 pointGiven, TVector3 &pointOnHelix, Double_t &alpha) const;

    /**
     * Extrapolate track due to y-position of the given point.
     * Returns extrapolation length from the initial track reference position.
     *
     * @param pointGiven    given point.
     * @param pointOnHelix  extrapolated position on the helix.
     * @param alpha         extrapolated alpha angle.
    */
    Double_t ExtrapolateToPointY(TVector3 pointGiven, TVector3 &pointOnHelix, Double_t &alpha) const;

    /** Check feasibility of Extrapolation of track to zy-plane at x-position. */
    bool CheckExtrapolateToX(Double_t x) const;

    /** Check feasibility of Extrapolation of track to xy-plane at z-position. */
    bool CheckExtrapolateToZ(Double_t z) const;

    /**
     * Extrapolate track to zy-plane at x-position.
     * Returns true if extrapolation is possible, false if not.
     *
     * @param x              x-position of zy-plane.
     * @param pointOnHelix1  extrapolated position on the helix close to fAlphaHead
     * @param alpha1         extrapolated alpha angle close to fAlphaHead
     * @param pointOnHelix2  extrapolated position on the helix close to fAlphaTail
     * @param alpha2         extrapolated alpha angle close to fAlphaTail
    */
    bool ExtrapolateToX(Double_t x, 
        TVector3 &pointOnHelix1, Double_t &alpha1,
        TVector3 &pointOnHelix2, Double_t &alpha2) const;

    /**
     * Extrapolate track to xy-plane at z-position.
     * Returns true if extrapolation is possible, false if not.
     *
     * @param z              z-position of xy-plane.
     * @param pointOnHelix1  extrapolated position on the helix close to fAlphaHead
     * @param alpha1         extrapolated alpha angle close to fAlphaHead
     * @param pointOnHelix2  extrapolated position on the helix close to fAlphaTail
     * @param alpha2         extrapolated alpha angle close to fAlphaTail
    */
    bool ExtrapolateToZ(Double_t z,
        TVector3 &pointOnHelix1, Double_t &alpha1,
        TVector3 &pointOnHelix2, Double_t &alpha2) const;

    /**
     * Extrapolate track to zy-plane at x-position.
     * Returns true if extrapolation is possible, false if not.
     *
     * @param x              x-position of zy-plane.
     * @param alphaRef       reference position for extapolation
     * @param pointOnHelix   extrapolated position on the helix close to alphaRef
    */
    bool ExtrapolateToX(Double_t x, Double_t alphaRef, TVector3 &pointOnHelix) const;

    /**
     * Extrapolate track to xy-plane at z-position.
     * Returns true if extrapolation is possible, false if not.
     *
     * @param z              z-position of xy-plane
     * @param alphaRef       reference position for extapolation
     * @param pointOnHelix   extrapolated position on the helix close to alphaRef
    */
    bool ExtrapolateToZ(Double_t z, Double_t alphaRef, TVector3 &pointOnHelix) const;

    /**
     * Extrapolate track to xy-plane at z-position.
     * Returns true if extrapolation is possible, false if not.
     *
     * @param z             z-position of xy-plane.
     * @param pointOnHelix  extrapolated position close to tracjectory.
    */
    bool ExtrapolateToZ(Double_t z, TVector3 &pointOnHelix) const;

    bool ExtrapolateToX(Double_t x, TVector3 &pointOnHelix) const;

    /** 
     * Extrapolate head of track about length
     * Return extrapolated position
    */
    TVector3 ExtrapolateHead(Double_t length) const;

    /** 
     * Extrapolate tail of track about length
     * Return extrapolated position
    */
    TVector3 ExtrapolateTail(Double_t length) const;

    /**
     * Interpolate between head and tail of helix.
     * Return interpolated position using ratio
     * Interpolate when 0 < r < 1.
     * Extrapolate when r < 0 or r > 1.
    */
    TVector3 InterpolateByRatio(Double_t r) const;

    /**
     * Interpolate between head and tail of helix.
     * Return interpolated position using length
     * Interpolate when 0 < length < TrackLength().
     * Extrapolate when length < 0 or length > TrackLength().
    */
    TVector3 InterpolateByLength(Double_t r) const;

    /**
     * Map and return position.
     * Concept of this mapping is to strecth helix to straight line.
     * - 1st axis : radial axis
     * - 2nd axis : normal to 1st and 3rd axis
     * - 3nd axis : length along helix line
    */
    TVector3 Map(TVector3 p) const;

    /**
     * Extrapolate to cloesest position from p to helix by Mapping.
     * Return length
     * @param p  input position
     * @param q  position on helix.
     * @param m  mapped position
    */
    Double_t ExtrapolateByMap(TVector3 p, TVector3 &q, TVector3 &m) const;

    /**
     * Check continuity of the track. Hit array must be filled.
     * Returns ratio of the continuous region. (-1 if less than 2 hits)
     *
     * If difference between "length from hits" is smaller than 20,
     * (> pad-diagonal, < 2*pad-row) the region is considered to be connected.
     *
     * The total length calculation is different from TrackLength(),
     * because TrackLength() use only the head/tail alpha angle informatiotn
     * but this method calculate length directly from the hit using
     * Map(...) method. 
     *
     * @param continuousLength  calculated length of continuos region.
     * @param totalLength       calculated total length of the track.
    */
    Double_t Continuity(Double_t &totalLength, Double_t &continuousLength);

    /** Same as Continuity(Double_t&, Double_t&) but no length outputs. */
    Double_t Continuity();


  ClassDef(STHelixTrack, 5)
};

class STHitByDistanceTo
{
  private:
    TVector3 fP;

  public:
    STHitByDistanceTo(TVector3 p):fP(p) {}
    bool operator() (STHit* a, STHit* b) {
      return (a->GetPosition()-fP).Mag() < (b->GetPosition()-fP).Mag();
    }
};

class STHitSortByIncreasingLength
{
  private:
    STHelixTrack *fTrk;

  public:
    STHitSortByIncreasingLength(STHelixTrack *track):fTrk(track) {}
    bool operator() (STHit* a, STHit* b) {
      return fTrk->Map(a->GetPosition()).Z() < fTrk->Map(b->GetPosition()).Z();
    }
};

class STHitSortByDecreasingLength
{
  private:
    STHelixTrack *fTrk;

  public:
    STHitSortByDecreasingLength(STHelixTrack *track):fTrk(track) {}
    bool operator() (STHit* a, STHit* b) {
      return fTrk->Map(a->GetPosition()).Z() > fTrk->Map(b->GetPosition()).Z();
    }
};

#endif
