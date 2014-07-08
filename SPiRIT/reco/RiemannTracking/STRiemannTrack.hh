//-----------------------------------------------------------------
// Description:
//      Track on Riemann Sphere
//      Circle parameters can be calculated from plane parameters
//      plane(c,nx,ny,nz);
//
// Environment:
//      Software developed for the SpiRIT-TPC at RIBF-RIKEN
//
// Original Author List:
//      Sebastian Neubert    TUM
//
// Author List:
//      Genie Jhang          Korea University
//-----------------------------------------------------------------

#ifndef STRIEMANNTRACK_HH
#define STRIEMANNTRACK_HH

// SpiRITROOT classes


// ROOT classes
#include "TObject.h"
#include "TVector3.h"

// STL
#include <list>

class TVector3;
class STRiemannHit;

class STRiemannTrack : public TObject
{
  public:
    STRiemannTrack();
    STRiemannTrack(Double_t scale);
    ~STRiemannTrack() // does NOT delete Riemann Hits

    void  SetVerbose(Bool_t value = kTRUE);
    void  SetSort(Bool_t value = kTRUE);
    void  SetFinished(Bool_t value = kTRUE);
    void  SetGood(Bool_t opt = kTRUE);

    void  DeleteHits(); // deletes Riemann Hits

    const TVector3 &GetN()               const;
          Double_t  GetC()               const;
    const TVector3 &GetAv()              const;
          Double_t  GetRiemannScale()    const;

          Double_t  GetR()               const;
    const TVector3 &GetCenter()          const;
          Double_t  GetDip()             const;
          Double_t  GetSinDip()          const;

            Bool_t  IsFitted()           const;
            Bool_t  IsInitialized()      const;
            Bool_t  IsFinished();
            Bool_t  IsGood();

          Double_t  GetM()               const;
          Double_t  GetT()               const;
 
          Double_t  GetResolution()      const; // approximate momentum resolution of the track
          Double_t  GetQuality()         const; // gives a quality-estimate of the track in range [0; 1], the higher the better!


            UInt_t  GetNumHits()                   const;
      STRiemannHit *GetHit(UInt_t iHit)            const;
      STRiemannHit *GetLastHit()                   const;
      STRiemannHit *GetFirstHit()                  const;
      const std::vector<STRiemannHit *> *GetHits() const;

             Int_t  GetClosestHit(STRiemannHit *hit, Double_t &Dist, TVector3 &dir) const;
             Int_t  GetClosestHit(STRiemannHit *hit, Double_t &Dist, Int_t from = 0, Int_t to = 10000000) const; // optional: range where to search for closest hit
             Int_t  GetClosestRiemannHit(STRiemannHit *hit, Double_t &Dist)         const;

    // these functions can be used to calculate seed values for the fitter
              void  GetPosDirOnHelix(UInt_t i, TVector3 &pos, TVector3 &dir)        const; // get position (3D) of point i on the fitted helix
          Double_t  GetMom(Double_t Bz)      const; // get the magnitude of the momentum, Bz is the z component in kGauss
             Int_t  IsWinding()              const; // +- 1; winding sense along z axis

          TVector3  pocaToZ()                const; // calc POCA to Z-Axis
          TVector3  pocaToIP(Double_t z = 0) const; // calc POCA to (0,0,z)

              void  AddHit(STRiemannHit *hit);
              void  RemoveHit(UInt_t ihit);


              void  InitTargetTrack(Double_t Dip, Double_t curvature = 0); // init as straight track from Origin for single hit track
              void  InitCircle(Double_t phi); // init as a circle

          
    // Operations ----------------------
              void  FitAndSort();  // refit the plane and sort the hits; calculate center and radius

    // RMS of distances of hits to intersection of plane with riemann sphere
          Double_t  PlaneRMS() const { return fRms; }
          Double_t  DistRMS()  const;

    // calculate distance of hit to intersection of plane with riemann sphere
    // if use Arguments == kFALSE, the members fN and fC will be used for calculation
          Double_t  Dist(STRiemannHit *hit,
                             TVector3  n2 = TVector3(0., 0., 0.),
                             Double_t  c2 = 0,
                               Bool_t  useArguments = kFALSE)     const;

          Double_t  DistHelix(STRiemannHit *hit,
                                    Bool_t  calcPos = kTRUE,
                                    Bool_t  TwoPiCheck = kFALSE,
                                   Vector3 *POCA = NULL)          const; // distance to helix
             void   Plot(Bool_t standalone = kTRUE);

  private:
    // Operations ----------------------
    void Refit(); // refit the plane and dip; set angles of hits; calc rms
    void CenterR(); // calculate center and radius
    Double_t calcRMS(TVector3 n1, Double_t c1) const;

    // Private Data Members ------------
    TVector3 fN;  // normal vector of plane (pointing towards origin!)
    Double_t fC;     // distance of plane to origin

    TVector3 fCenter; // center of helix in 3D (z=0)
    Double_t fRadius;   // radius of helix

    Double_t fM; //parameters of Helix sz-fit: z = fM*phi + fT
    Double_t fT; //parameters of Helix sz-fit: z = fM*phi + fT

    Double_t fDip; // dip angle of track [0, pi]
    Double_t fSinDip; // sinus of dip angle

    Double_t fRms; // RMS of distances of hits to intersection of plane with riemann sphere

    Bool_t fIsFitted; // fitted plane and dip
    Bool_t fIsInitialized; // initSL was called, is set to kFALSE if fitandsort is called

    Bool_t fIsFinished; // track is finished, no more hits will be added
    Bool_t fIsGood; // track cannot be deleted by PR

    Bool_t fVerbose;

    Double_t fRiemannScale;

    std::vector<STRiemannHit *> fHits; // riemann hits of the track; track has ownership!
    TVector3 fAv;  // average over all hits
    Double_t fSumOfWeights; // for weighing the average with cluster error

    Bool_t fDoSort; // flag for switching on and off sorting


    // Private Methods -----------------
    Bool_t CheckScale(STRiemannHit *) const;

    Bool_t SortByZ(STRiemannHit *hit1, STRiemannHit *hit2);
    Bool_t SortByZInv(STRiemannHit *hit1, STRiemannHit *hit2);
    Bool_t SortByAngle(STRiemannHit *hit1, STRiemannHit *hit2);
    Bool_t SortByAngleInv(STRiemannHit *hit1, STRiemannHit *hit2);

  ClassDef(STRiemannTrack, 1)
};

#endif
