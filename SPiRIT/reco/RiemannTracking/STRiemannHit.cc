//-----------------------------------------------------------------
// Description:
//      Conformal Mapping of a hit
//      (x,y) -> (r,phi)-> riemann sphere
//
// Environment:
//      Software developed for the SpiRIT-TPC at RIBF-RIKEN
//
// Original Author:
//      Sebastian Neubert    TUM
//
// Author List:
//      Genie Jhang          Korea University
//-----------------------------------------------------------------

// SpiRITROOT classes
#include "STRiemannHit.hh"
#include "STRiemannTrack.hh"

// STL
#include <cmath>

// ROOT classes
#include "TMath.h"

ClassImp(STRiemannHit)

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

//  fCluster = NULL;
  fEvent = NULL;
  fRiemannScale = riemannScale;
}

STRiemannHit::STRiemannHit(STevent* event, Double_t riemannScale)
{
  InitVariables(0, 0, riemannScale);

//  fCluster = cluster;
  fEvent = event;
  fRiemannScale = riemannScale;

/*
  TVector2 proj(cluster -> pos().X(), cluster -> pos().Y());
  Double_t r = proj.Mod()/fRiemannScale;
  Double_t phi = proj.Phi();
  

  InitVariables(r, phi, riemannScale);
  */
}


STRiemannHit::~STRiemannHit()
{
}

void STRiemannHit::InitVariables(Double_t r, Double_t phi, Double_t riemannScale)
{
  fS = 0;
  fAngleOnHelix = 0;
  fRiemannScale = riemannScale;

  Double_t r2 = r*r;
  Double_t d = 1 + r2;

  fX.SetX(r*cos(phi)/d);
  fX.SetY(r*sin(phi)/d);
  fX.SetZ(r2/d);

  fLogger = FairLogger::GetLogger();
}

// Simple getter and setter methods -----------------------------------------------------------
const TVector3 &STRiemannHit::GetX()             const        { return fX; }
//     STCluster *STRiemannHit::GetCluster()       const        { return fCluster; }
       STEvent *STRiemannHit::GetEvent()         const        { return fEvent; }
      Double_t  STRiemannHit::GetS()             const        { return fS; }
      Double_t  STRiemannHit::GetAngleOnHelix()  const        { return fAngleOnHelix; }
      Double_t  STRiemannHit::GetRiemannScale()  const        { return fRiemannScale; }

      Double_t  STRiemannHit::SetS(Double_t value)            { fS = value; }
      Double_t  STRiemannHit::SetAngleOnHelix(Double_t value) { fAngleOnHelix = value; }
// --------------------------------------------------------------------------------------------

Double_t
STRiemannHit::GetZ() const
{
  /*
  if(fCluster == NULL){
    fLogger -> Warning(MESSAGE_ORIGIN, "No cluster available!");

    return 0;
  }
  */

  return 0; //fCluster -> pos().Z(); <<-- to be modified
}

