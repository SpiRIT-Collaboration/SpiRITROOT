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

// SpiRITROOT classes
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"

// STL
#include <cmath>

// ROOT classes
#include "TMath.h"

ClassImp(STRiemannHit);

STRiemannHit::STRiemannHit()
{
  InitVariables(0, 0);
}

STRiemannHit::STRiemannHit(Double_t riemannScale)
{
  InitVariables(0, 0, riemannScale);
}

STRiemannHit::STRiemannHit(Double_t r, Double_t phi, Double_t riemannScale)
{
  InitVariables(r, phi, riemannScale);

  fRiemannScale = riemannScale;
}

STRiemannHit::STRiemannHit(STHit *hit, Double_t riemannScale)
{
  TVector2 proj(hit -> GetPosition().X(), hit -> GetPosition().Y());
  Double_t r = proj.Mod()/riemannScale;
  Double_t phi = proj.Phi();
  
  InitVariables(r, phi, riemannScale);

  fHit = hit;
}


STRiemannHit::~STRiemannHit()
{
}

void STRiemannHit::InitVariables(Double_t r, Double_t phi, Double_t riemannScale)
{
  fHit = NULL;

  Double_t r2 = r*r;
  Double_t d = 1 + r2;

  fX.SetX(r*cos(phi)/d);
  fX.SetY(r*sin(phi)/d);
  fX.SetZ(r2/d);

  fS = 0;
  fAngleOnHelix = 0;
  fRiemannScale = riemannScale;

  fLogger = FairLogger::GetLogger();
}

// Simple getter and setter methods -----------------------------------------------------------
const TVector3 &STRiemannHit::GetX()             const        { return fX; }
         STHit *STRiemannHit::GetHit()           const        { return fHit; }
      Double_t  STRiemannHit::GetS()             const        { return fS; }
      Double_t  STRiemannHit::GetAngleOnHelix()  const        { return fAngleOnHelix; }
      Double_t  STRiemannHit::GetRiemannScale()  const        { return fRiemannScale; }

      void      STRiemannHit::SetS(Double_t value)            { fS = value; }
      void      STRiemannHit::SetAngleOnHelix(Double_t value) { fAngleOnHelix = value; }
// --------------------------------------------------------------------------------------------

Double_t
STRiemannHit::GetZ() const
{
  if(fHit == NULL){
    fLogger -> Warning(MESSAGE_ORIGIN, "No hit available!");

    return 0;
  }

  return fHit -> GetPosition().Z();
}
