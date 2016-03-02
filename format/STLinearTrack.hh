/**
 * @brief SpiRITROOT linear track container 
 *
 * @author JungWoo Lee
 */

#ifndef STLINEARTRACK
#define STLINEARTRACK

#include "STHit.hh"

#include "TObject.h"
#include "TVector3.h"

#include <vector>

class STLinearTrack : public TObject
{
  public:
    STLinearTrack();
    STLinearTrack(STLinearTrack *track);
    STLinearTrack(Int_t trackID, STHit* hit);
    virtual ~STLinearTrack();

    virtual void Reset() { Clear(); }
    virtual void Clear(Option_t *option = "");

    virtual void DeleteHits();

    virtual void AddHit(STHit *hit); ///< Add hit and update parameters

    void SetTrackID(Int_t id);       ///< Set track ID
    void SetIsPrimary(Bool_t val);   ///< Set track primary flag
    void SetIsFitted(Bool_t val);    ///< Set fitted flag
    void SetIsSorted(Bool_t val);    ///< Set sorted flag

    void SetVertex(Int_t ith, TVector3 pos);    ///< Set position of vextex
    void SetDirection(TVector3 vec); ///< Set direction vector
    void SetNormal(TVector3 vec);    ///< Set normal vector
    void SetCentroid(TVector3 pos);  ///< Set centroid

    void SetXVertex(Int_t ith, Double_t x);     ///< Set x position of vextex
    void SetYVertex(Int_t ith, Double_t y);     ///< Set y position of vextex
    void SetZVertex(Int_t ith, Double_t z);     ///< Set z position of vextex

    void SetXDirection(Double_t x);  ///< Set x component of direction vector
    void SetYDirection(Double_t y);  ///< Set y component of direction vector
    void SetZDirection(Double_t z);  ///< Set z component of direction vector

    void SetXNormal(Double_t x);     ///< Set x component of normal vector
    void SetYNormal(Double_t y);     ///< Set y component of normal vector
    void SetZNormal(Double_t z);     ///< Set z component of normal vector

    void SetChargeSum(Double_t val);  
    void SetNumHits(Int_t val);

    void SetRMSLine (Double_t rms);
    void SetRMSPlane(Double_t rms);


       Int_t GetTrackID()    const; ///< Get track ID
      Bool_t IsPrimary()     const; ///< Get track primary flag
      Bool_t IsFitted()      const; ///< Get fitted flag
      Bool_t IsSorted()      const; ///< Get fitted flag

    TVector3 GetVertex(Int_t ith)     const; ///< Get position of vextex
    TVector3 GetDirection()  const; ///< Get direction vector
    TVector3 GetNormal()     const; ///< Get normal vector
    TVector3 GetCentroid()   const; ///< Get centroid

    Double_t GetXVertex(Int_t ith)    const; ///< Get x position of vextex
    Double_t GetYVertex(Int_t ith)    const; ///< Get y position of vextex
    Double_t GetZVertex(Int_t ith)    const; ///< Get z position of vextex

    Double_t GetXDirection() const; ///< Get x component of direction vector
    Double_t GetYDirection() const; ///< Get y component of direction vector
    Double_t GetZDirection() const; ///< Get z component of direction vector

    Double_t GetXNormal()    const; ///< Get x component of normal vector
    Double_t GetYNormal()    const; ///< Get y component of normal vector
    Double_t GetZNormal()    const; ///< Get z component of normal vector

    Double_t GetXCentroid()  const; ///< Get x component of centroid
    Double_t GetYCentroid()  const; ///< Get y component of centroid
    Double_t GetZCentroid()  const; ///< Get z component of centroid

    Double_t GetChargeSum()  const; ///< Get charge sum 
    virtual Int_t GetNumHits();     ///< Get number of hits
    virtual Int_t GetNumHitsRemoved();

    std::vector<Int_t>  *GetHitIDArray();      ///< Get hit ID array
    std::vector<STHit*> *GetHitPointerArray(); ///< Get hit ID array

    Int_t  GetHitID(Int_t i);
    STHit *GetHit(Int_t i);

    Double_t GetRMSLine()    const;
    Double_t GetRMSPlane()   const;

    Double_t GetSumDistCXX() const; ///< SUM_i {(x_centroid-x_i)*(x_centroid-x_i) }
    Double_t GetSumDistCYY() const; ///< SUM_i {(y_centroid-y_i)*(y_centroid-y_i) }
    Double_t GetSumDistCZZ() const; ///< SUM_i {(z_centroid-z_i)*(z_centroid-z_i) }

    Double_t GetSumDistCXY() const; ///< SUM_i {(x_centroid-x_i)*(y_centroid-y_i) }
    Double_t GetSumDistCYZ() const; ///< SUM_i {(y_centroid-y_i)*(z_centroid-z_i) }
    Double_t GetSumDistCZX() const; ///< SUM_i {(z_centroid-z_i)*(x_centroid-x_i) }

    Double_t GetExpectationX()  const;
    Double_t GetExpectationY()  const;
    Double_t GetExpectationZ()  const;

    Double_t GetExpectationXX() const;
    Double_t GetExpectationYY() const;
    Double_t GetExpectationZZ() const;

    Double_t GetExpectationXY() const;
    Double_t GetExpectationYZ() const;
    Double_t GetExpectationZX() const;

  protected:
    Int_t  fTrackID;   ///< Track ID.
    Bool_t fIsPrimary; ///< True if track is primary track.
    Bool_t fIsFitted;  ///< True if fitted.
    Bool_t fIsSorted;  ///< True if sorted.

    Double_t fXVertex[2]; ///< x position of vextex
    Double_t fYVertex[2]; ///< y position of vextex
    Double_t fZVertex[2]; ///< z position of vextex

    Double_t fXDirection; ///< x component of direction vector
    Double_t fYDirection; ///< y component of direction vector
    Double_t fZDirection; ///< z component of direction vector

    Double_t fXNormal; ///< x component of normal vector
    Double_t fYNormal; ///< y component of normal vector
    Double_t fZNormal; ///< z component of normal vector

    Double_t fChargeSum; ///< charge sum
       Int_t fNumHits;

    std::vector<STHit*> fHitPtrArray; //-> < STHit pointer array
    std::vector<Int_t>  fHitIDArray;  //-> < hit id array

    Double_t fRMSLine;
    Double_t fRMSPlane;

    // For track fit ___________________________________________________________

    Double_t fExpectationX; ///< Expectation value of x
    Double_t fExpectationY; ///< Expectation value of y
    Double_t fExpectationZ; ///< Expectation value of z

    Double_t fExpectationXX; ///< Expectation value of x^2
    Double_t fExpectationYY; ///< Expectation value of y^2
    Double_t fExpectationZZ; ///< Expectation value of z^2

    Double_t fExpectationXY; ///< Expectation value of xy
    Double_t fExpectationYZ; ///< Expectation value of yz
    Double_t fExpectationZX; ///< Expectation value of zx


  ClassDef(STLinearTrack, 8);
};

#endif
