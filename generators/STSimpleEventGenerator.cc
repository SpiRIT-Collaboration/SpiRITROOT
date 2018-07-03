/**
 * @brief SPiRIT Simple Event Generator
 * @author JungWoo Lee (Korea Univ.)
 * modified by Mizuki Nishimura(RIKEN) 
 */

#include "STSimpleEventGenerator.hh"
#include "TSystem.h"
#include <iostream>

using namespace std;

ClassImp(STSimpleEventGenerator);

STSimpleEventGenerator::STSimpleEventGenerator()
:FairGenerator(),
  fPDG(0),
  fV3Vertex(TVector3(0,0,0)),
  fPDirection(TVector3(0,0,0)),
  fNEvents(0),
  fCurrentEvent(0),
  fMultiplicity(0),
  fPList(0),
  fEventMode(0)
{
}

STSimpleEventGenerator:: STSimpleEventGenerator(Int_t pdg, Int_t numP, Double_t *listP, Int_t mult, Double_t x0, Double_t y0, Double_t z0, Double_t vx, Double_t vy, Double_t vz)
:FairGenerator("STSimpleEventGenerator"),
  fPDG(pdg),
  fV3Vertex(TVector3(x0,y0,z0)),
  fCurrentEvent(0),
  fMultiplicity(mult),
  fNEvents(numP*mult)
{
  SetMomentumDirection(vx,vy,vz);

  fPList = new Double_t[numP];
  for (Int_t iP = 0; iP < numP; iP++)
    fPList[iP] = listP[iP];

  fEventMode = 1;
}

STSimpleEventGenerator::~STSimpleEventGenerator()
{
}

Bool_t STSimpleEventGenerator::SetAngleStep(Int_t  pdg, UInt_t numEvt, Double_t p, Double_t theta_begin, Double_t theta_end, Double_t phi_begin, Double_t phi_end)
{
  if( numEvt == 0 ) return kFALSE;

  fPDG = pdg;

  theta_begin = theta_begin* TMath::Pi()/180.;
  theta_end   = theta_end  * TMath::Pi()/180.;
  phi_begin   = phi_begin  * TMath::Pi()/180.;
  phi_end     = phi_end    * TMath::Pi()/180.;



  Double_t d_theta = (theta_end - theta_begin)/(Double_t)numEvt;
  Double_t d_phi   = (phi_end   - phi_begin)  /(Double_t)numEvt;

  for( UInt_t ip = 0; ip < numEvt; ip++ ){

    Double_t theta = ip * d_theta + theta_begin;
    Double_t phi   = ip * d_phi   + phi_begin;

    TVector3 vp(0, 0., p);
    vp.SetTheta(theta);
    vp.SetPhi(phi);

    fMomentum.push_back(vp);

  } 

  fNEvents = numEvt;

  fEventMode = 2;
  return kTRUE;
}


Bool_t
STSimpleEventGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{

  TVector3 p;

  switch( fEventMode ) {
    case 1:
      p.SetMag(fPList[fCurrentEvent++/fMultiplicity]);
      p = fPDirection;

      break;

    case 2:

      p = fMomentum.at(fCurrentEvent);
      fCurrentEvent++;

      break;

    default:

      return kFALSE;
  }

  primGen->AddTrack(fPDG,p.X(),p.Y(),p.Z(),fV3Vertex.X(),fV3Vertex.Y(),fV3Vertex.Z());

  return kTRUE;
}

