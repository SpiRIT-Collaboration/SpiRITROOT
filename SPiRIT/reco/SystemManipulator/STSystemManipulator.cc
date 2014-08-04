//-----------------------------------------------------------
// Description:
//   Translate the system for the vertex to be origin
//   and rotate it for the track to make circle on xy plane
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

// SpiRITROOT classes
#include "STSystemManipulator.hh"

ClassImp(STSystemManipulator);

STSystemManipulator::STSystemManipulator()
{
  /**
    * Default **fTrans** is moving the hits to our origin from collision point. <br>
    * Default **fAxis** and **fAngle** is rotating the system so that the helix draws circle on xy plane <br>
    * and advances along z direction. That is, parameterize the helix with \f$[ \rho\cos(\phi+\phi_0), \rho\sin(\phi-\phi_0), m(\phi-\phi_0) ] \f$
   **/

  fTrans = TVector3(0., 213.3, 35.2);
  fAxis = 0;
  fAngle = 90./180.*TMath::Pi();
}

STSystemManipulator::~STSystemManipulator()
{}

void
STSystemManipulator::Change(TVector3 &vector)
{
  Translate(vector, fTrans);
  Rotate(vector, fAxis, fAngle);
}

void
STSystemManipulator::Restore(TVector3 &vector)
{
  Rotate(vector, fAxis, -fAngle);
  Translate(vector, -fTrans);
}

void
STSystemManipulator::Translate(TVector3 &vector, TVector3 trans)
{
  vector += trans;
}

void
STSystemManipulator::Rotate(TVector3 &vector, Int_t axis, Double_t angle)
{
  TVector3 axisVec(0, 0, 0);
  axisVec[axis]++;

  vector.Rotate(angle, axisVec);
}
