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
#include "STHit.hh"
#include "STHitCluster.hh"

// STL
#include <vector>

using std::vector;

ClassImp(STSystemManipulator);

STSystemManipulator::STSystemManipulator()
{
  /**
    * Default **fTrans** is moving the hits to our origin from collision point.
   **/

  fTrans = TVector3(0., 213.3, 35.2);
}

STSystemManipulator::~STSystemManipulator()
{}

STEvent *
STSystemManipulator::Change(STEvent *event)
{
  if (event -> IsChanged())
    return event;

  Bool_t isClustered = event -> IsClustered();
  Bool_t isTracked = event -> IsTracked();
  STEvent *newEvent = new STEvent(event);
  newEvent -> SetIsChanged(kTRUE);

  Int_t numHits = newEvent -> GetNumHits();
  for (Int_t iHit = 0; iHit < numHits; iHit++) {
    STHit *hit = newEvent -> GetHit(iHit);

    TVector3 pos = hit -> GetPosition();
    Translate(pos, fTrans);
    Exchange(pos);
    hit -> SetPosition(pos);

    TVector3 posSigma = hit -> GetPosSigma();
    Exchange(posSigma);
    hit -> SetPosSigma(posSigma);
  }

  if (isClustered) {
    Int_t numClusters = newEvent -> GetNumClusters();
    for (Int_t iCluster = 0; iCluster < numClusters; iCluster++) {
      STHitCluster *cluster = newEvent -> GetCluster(iCluster);

      TVector3 pos = cluster -> GetPosition();
      Translate(pos, fTrans);
      Exchange(pos);
      cluster -> SetPosition(pos);

      TVector3 posSigma = cluster -> GetPosSigma();
      Exchange(posSigma);
      cluster -> SetPosSigma(posSigma);

      TMatrixD covMatrix = cluster -> GetCovMatrix();
      Exchange(covMatrix);
      cluster -> SetCovMatrix(covMatrix);
    }
  }

  if (isTracked) {
  }

  return newEvent;
}

STEvent *
STSystemManipulator::Restore(STEvent *event)
{
  return NULL;
}

void
STSystemManipulator::Translate(TVector3 &vector, TVector3 trans)
{
  vector += trans;
}

void
STSystemManipulator::Exchange(TVector3 &vector)
{
  Double_t x = vector[0];
  Double_t y = vector[1];
  Double_t z = vector[2];

  vector = TVector3(z, x, y);
}

void
STSystemManipulator::Exchange(TMatrixD &matrix)
{
  Double_t xx = matrix(0, 0);
  Double_t yy = matrix(1, 1);
  Double_t zz = matrix(2, 2);
  Double_t xy = matrix(0, 1);
  Double_t xz = matrix(0, 2);
  Double_t yz = matrix(1, 2);

  matrix(0, 0) = zz;
  matrix(1, 1) = xx;
  matrix(2, 2) = yy;
  matrix(0, 1) = xz;
  matrix(0, 2) = yz;
  matrix(1, 2) = xy;

  matrix(1, 0) = matrix(0, 1);
  matrix(2, 0) = matrix(0, 2);
  matrix(2, 1) = matrix(1, 2);
}
