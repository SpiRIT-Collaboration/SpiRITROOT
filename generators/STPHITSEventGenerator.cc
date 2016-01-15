/**
 * @brief SPiRIT PHITS Event Generator
 * @author Genie Jhang (Korea Univ.)
 */

// SpiRITROOT
#include "STPHITSEventGenerator.hh"

// ROOT
#include "TSystem.h"
#include "TVector3.h"

// STL
#include <iostream>

ClassImp(STPHITSEventGenerator);

STPHITSEventGenerator::STPHITSEventGenerator()
: FairGenerator("STPHITSEventGenerator"),
  fDataFile(NULL),
  fDataTree(NULL),
  fEventArray(NULL),
  fVx(0), fVy(0), fVz(0),
  fNumEvents(0),
  fCurrentEvent(0),
  fSingleTrackEventFlag(kFALSE),
  fCurrentParticle(0)
{
}

void
STPHITSEventGenerator::SetData(TString filename, TString treename)
{
  if (fDataFile != NULL) {
    delete fDataFile;

    fDataFile = NULL;
  }

  fDataFile = new TFile(filename);

  if (fDataTree != NULL) {
    delete fDataTree;

    fDataTree = NULL;
  }

  fDataTree = (TTree *) fDataFile -> Get(treename);
  fDataTree -> SetBranchAddress("fparts", &fEventArray);

  fNumEvents = fDataTree -> GetEntries();
}

Bool_t
STPHITSEventGenerator::ReadEvent(FairPrimaryGenerator* primGen)
{
  if (fSingleTrackEventFlag == kTRUE)
  {
    if (fEventArray == NULL)
      fDataTree -> GetEntry(fCurrentEvent);

    Int_t numParticles = fEventArray -> GetEntries();

    if (numParticles == 1 || fCurrentParticle == numParticles) {
      fDataTree -> GetEntry(++fCurrentEvent);
      fCurrentParticle = 0;
      return ReadEvent(primGen);
    }

    Particle *particle = (Particle *) fEventArray -> At(fCurrentParticle++);

    if (particle -> kf >= 1000002)
      ReadEvent(primGen);
      
    Int_t pdg = kfToPDG(particle -> kf);
    Double_t mass = particle -> m;
    Double_t ke = particle -> ke;
    Double_t pMag = sqrt(ke*(ke + 2*mass))/1000.; // in GeV/c

    TVector3 p(particle -> mom[0], particle -> mom[1], particle -> mom[2]);
    p.SetMag(pMag);

    TVector3 pos(particle -> pos[0]/10., 
        particle -> pos[1]/10., 
        particle -> pos[2]/10.);
    pos += TVector3(fVx, fVy, fVz);

    primGen -> AddTrack(pdg, p.X(), p.Y(), p.Z(), pos.X(), pos.Y(), pos.Z());

    return kTRUE;
  }

  fDataTree -> GetEntry(fCurrentEvent++);

  Int_t numParticles = fEventArray -> GetEntries();
  if (numParticles == 1)
    return ReadEvent(primGen);

  for (Int_t iPart = 0; iPart < numParticles; iPart++) {
    Particle *particle = (Particle *) fEventArray -> At(iPart);

    Int_t pdg = kfToPDG(particle -> kf);
    Double_t mass = particle -> m;
    Double_t ke = particle -> ke;
    Double_t pMag = sqrt(ke*(ke + 2*mass))/1000.; // in GeV/c

    TVector3 p(particle -> mom[0], particle -> mom[1], particle -> mom[2]);
    p.SetMag(pMag);

    TVector3 pos(particle -> pos[0]/10., particle -> pos[1]/10., particle -> pos[2]/10.);
    pos += TVector3(fVx, fVy, fVz);

    primGen -> AddTrack(pdg, p.X(), p.Y(), p.Z(), pos.X(), pos.Y(), pos.Z());
  }

  return kTRUE;
}

void
STPHITSEventGenerator::SetPrimaryVertex(Double_t x, Double_t y, Double_t z)
{
  fVx = x;
  fVy = y;
  fVz = z;
}

Long64_t
STPHITSEventGenerator::GetNumEvents()
{
  return fNumEvents;
}

Long64_t
STPHITSEventGenerator::GetCurrentEvent()
{
  return fCurrentEvent - 1;
}

void
STPHITSEventGenerator::SetSingleTrackEvent()
{
  fSingleTrackEventFlag = kTRUE;
}

void
STPHITSEventGenerator::SetEvent(Int_t event)
{
  fCurrentEvent = event;
}

Int_t
STPHITSEventGenerator::GetNumTracks(Int_t event)
{
  if (fEventArray == NULL)
    return -1;

  fDataTree -> GetEntry(event);
  Int_t numTracks = fEventArray -> GetEntries();
  fDataTree -> GetEntry(++fCurrentEvent);

  return numTracks;
}

Int_t
STPHITSEventGenerator::kfToPDG(Long64_t kfCode)
{
  if (kfCode < 1000002)
    return kfCode;
  else {
    Int_t A = kfCode%1000000;
    Int_t Z = kfCode/1000000;

    return (1000000000 + Z*10000 + A*10);
  }
}
