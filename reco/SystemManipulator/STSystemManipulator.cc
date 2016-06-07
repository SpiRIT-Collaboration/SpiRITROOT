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

void STSystemManipulator::SetTrans(TVector3 trans) { fTrans = trans; }

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
    Translate(pos);
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
      Translate(pos);
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

void
STSystemManipulator::Change(TClonesArray *in, TClonesArray *out)
{
  Int_t numClusters = in -> GetEntriesFast();
  for (Int_t iCluster = 0; iCluster < numClusters; iCluster++) {
    STHitCluster *hit = (STHitCluster *) in -> At(iCluster);
    STHitCluster *smhit = new ((*out)[out->GetEntriesFast()]) STHitCluster(hit);

    TVector3 pos = smhit -> GetPosition();
    Translate(pos);
    Exchange(pos);
    smhit -> SetPosition(pos);

    TVector3 posSigma = smhit -> GetPosSigma();
    Exchange(posSigma);
    smhit -> SetPosSigma(posSigma);

    TMatrixD covMatrix = smhit -> GetCovMatrix();
    Exchange(covMatrix);
    smhit -> SetCovMatrix(covMatrix);
  }
}

STEvent *
STSystemManipulator::Restore(STEvent *event)
{
  if (!(event -> IsChanged()))
    return event;

  Bool_t isClustered = event -> IsClustered();
  Bool_t isTracked = event -> IsTracked();
  STEvent *newEvent = new STEvent(event);
  newEvent -> SetIsChanged(kFALSE);

  Int_t numHits = newEvent -> GetNumHits();
  for (Int_t iHit = 0; iHit < numHits; iHit++) {
    STHit *hit = newEvent -> GetHit(iHit);

    TVector3 pos = hit -> GetPosition();
    Exchange(pos, kTRUE);
    Translate(pos, kTRUE);
    hit -> SetPosition(pos);

    TVector3 posSigma = hit -> GetPosSigma();
    Exchange(posSigma, kTRUE);
    hit -> SetPosSigma(posSigma);
  }

  if (isClustered) {
    Int_t numClusters = newEvent -> GetNumClusters();
    for (Int_t iCluster = 0; iCluster < numClusters; iCluster++) {
      STHitCluster *cluster = newEvent -> GetCluster(iCluster);

      TVector3 pos = cluster -> GetPosition();
      Exchange(pos, kTRUE);
      Translate(pos, kTRUE);
      cluster -> SetPosition(pos);

      TVector3 posSigma = cluster -> GetPosSigma();
      Exchange(posSigma, kTRUE);
      cluster -> SetPosSigma(posSigma);

      TMatrixD covMatrix = cluster -> GetCovMatrix();
      Exchange(covMatrix, kTRUE);
      cluster -> SetCovMatrix(covMatrix);
    }
  }

  if (isTracked) {
  }

  return newEvent;
}

void
STSystemManipulator::Translate(TVector3 &vector, Bool_t restore)
{
  if (restore)
    vector -= fTrans;
  else
    vector += fTrans;
}

void
STSystemManipulator::Exchange(TVector3 &vector, Bool_t restore)
{
  Double_t x = vector[0];
  Double_t y = vector[1];
  Double_t z = vector[2];

  if (restore)
    vector = TVector3(y, z, x);
  else
    vector = TVector3(z, x, y);
}

void
STSystemManipulator::Exchange(TMatrixD &matrix, Bool_t restore)
{
  Double_t xx = matrix(0, 0);
  Double_t yy = matrix(1, 1);
  Double_t zz = matrix(2, 2);
  Double_t xy = matrix(0, 1);
  Double_t xz = matrix(0, 2);
  Double_t yz = matrix(1, 2);

  if (restore) {
    matrix(0, 0) = yy;
    matrix(1, 1) = zz;
    matrix(2, 2) = xx;
    matrix(0, 1) = yz;
    matrix(0, 2) = xy;
    matrix(1, 2) = xz;
  } else {
    matrix(0, 0) = zz;
    matrix(1, 1) = xx;
    matrix(2, 2) = yy;
    matrix(0, 1) = xz;
    matrix(0, 2) = yz;
    matrix(1, 2) = xy;
  }

  matrix(1, 0) = matrix(0, 1);
  matrix(2, 0) = matrix(0, 2);
  matrix(2, 1) = matrix(1, 2);
}
