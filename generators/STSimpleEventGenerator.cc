/**
 * @brief SPiRIT Simple Event Generator
 * @author JungWoo Lee (Korea Univ.)
 */

#include "STSimpleEventGenerator.hh"
#include "TSystem.h"
#include <iostream>

using namespace std;

ClassImp(STSimpleEventGenerator);

STSimpleEventGenerator::STSimpleEventGenerator()
: FairGenerator(),
  fPDG(0),
  fV3Vertex(TVector3(0,0,0)),
  fPDirection(TVector3(0,0,0)),
  fNEvents(0),
  fCurrentEvent(0),
  fMultiplicity(0),
  fPList(0)
{
}

STSimpleEventGenerator:: STSimpleEventGenerator(Int_t pdg, Int_t numP, Double_t *listP, Int_t mult, Double_t x0, Double_t y0, Double_t z0, Double_t vx, Double_t vy, Double_t vz)
: FairGenerator("STSimpleEventGenerator"),
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
}

STSimpleEventGenerator::~STSimpleEventGenerator()
{
}

Bool_t
STSimpleEventGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  Double_t pMag = fPList[fCurrentEvent++/fMultiplicity];
  TVector3 p(pMag*fPDirection);

  primGen->AddTrack(fPDG,p.X(),p.Y(),p.Z(),fV3Vertex.X(),fV3Vertex.Y(),fV3Vertex.Z());

  return kTRUE;
}
