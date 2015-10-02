/**
 * @brief  STRiemann Track
 *
 * @author Sebastian Neubert (TUM) -- original author for FOPIROOT
 * @author Genie Jhang (Korea University) -- implementation for SpiRITROOT
 * @author JungWoo Lee (Korea University) -- implementation for SpiRITROOT
 *
 * @detail Conformal Mapping of a hit
 *         (x,y) -> (r,phi)-> riemann sphere
 */

#ifndef STRIEMANNHIT_HH
#define STRIEMANNHIT_HH

// SpiRITROOT classes
#include "STHit.hh"

// ROOT classes
#include "TObject.h"
#include "TVector3.h"

// FairRoot classes
#include "FairLogger.h"

class STRiemannHit : public TObject
{
  public:
    STRiemannHit();
    STRiemannHit(Double_t riemannScale);
    STRiemannHit(Double_t r, Double_t phi, Double_t riemannScale = 86.1);
    STRiemannHit(STHit *hit, Double_t riemannScale = 86.1);
    ~STRiemannHit();

    const TVector3 &GetX() const;
    STHit *GetHit() const;
    Double_t GetS() const;
    Double_t GetAngleOnHelix() const;
    Double_t GetRiemannScale() const;
    Double_t GetZ() const;

    void SetS(Double_t value);
    void SetAngleOnHelix(Double_t value);

  private:
    void InitVariables(Double_t r, Double_t phi, Double_t riemannScale = 86.1);

    TVector3 fX;            ///< Position on Riemann sphere in cartesian coordinates
    STHit *fHit;            ///< STHit pointer, no ownership over this pointer!
    Double_t fS;            ///< Pathlength along track
    Double_t fAngleOnHelix; ///< Angle on helix
    Double_t fRiemannScale; ///< Scaling in xy plane: 8.7 for prototype, 24.6 for panda. What about our case?

    FairLogger *fLogger;    //! <

  ClassDef(STRiemannHit, 2);
};

#endif
