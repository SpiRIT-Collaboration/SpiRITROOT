/**
 * @brief SPiRIT Simple Event Generator
 * @author JungWoo Lee (Korea Univ.)
 * modified by Mizuki Nishimura(RIKEN) 
 */

#include "STSimpleEventGenerator.hh"
#include "FairRunSim.h"
#include "FairLogger.h"
#include "FairMCEventHeader.h"
#include "TSystem.h"
#include "TMath.h"
#include <iostream>

using namespace std;

ClassImp(STSimpleEventGenerator);

STSimpleEventGenerator::STSimpleEventGenerator()
:FairGenerator(),
  fPDG(0),
  fP(0),
  fV3Vertex(TVector3(0,0,0)),
  fPDirection(TVector3(0,0,0)),
  fNEvents(0),
  fCurrentEvent(0),
  fRealEvent(0),
  fMultiplicity(0),
  fPList(0),
  fEventMode(0)
{
  ran = new TRandom3;
}

STSimpleEventGenerator:: STSimpleEventGenerator(Int_t pdg, Int_t numP, Double_t *listP, Int_t mult, Double_t x0, Double_t y0, Double_t z0, Double_t vx, Double_t vy, Double_t vz)
:FairGenerator("STSimpleEventGenerator"),
  fPDG(pdg),
  fP(0),
  fV3Vertex(TVector3(x0,y0,z0)),
  fCurrentEvent(0),
  fRealEvent(0),
  fMultiplicity(mult),
 fNEvents(numP*mult)
{
  SetMomentumDirection(vx,vy,vz);

  fPList = new Double_t[numP];
  for (Int_t iP = 0; iP < numP; iP++)
    fPList[iP] = listP[iP];

  fEventMode = 1;
  ran = new TRandom3;
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

void
STSimpleEventGenerator::SetMCFile(TString fname, Int_t numEvt, Int_t pdg, Double_t pfactor)
{
  fEventMode = 3;
  fNEvents = numEvt;
  fin = new TFile(fname);
  gROOT->cd();
  ntp = (TNtuple*)fin->Get("mc");
  std::cout << "open mc file: " <<  fname << std::endl;
  fPDG = pdg;
  fP = pfactor;
  ntp->SetBranchAddress("px",&fX);
  ntp->SetBranchAddress("py",&fY);
  ntp->SetBranchAddress("pz",&fZ);
}

void
STSimpleEventGenerator::SetEventFile(TString fname)
{
  fin_eve = new TFile(fname);
  gROOT->cd();
  evetree = (TTree*)fin_eve->Get("evetree");
  std::cout << "open event file: " <<  fname << " " << evetree->GetEntries() << std::endl;
  evetree->SetBranchAddress("eventid",&fTreeEventID);
  evetree->SetBranchAddress("vtxx",&fVtxX);
  evetree->SetBranchAddress("vtxy",&fVtxY);
  evetree->SetBranchAddress("vtxz",&fVtxZ);

}

void
STSimpleEventGenerator::SetRealEvent(int eventid)
{
  fRealEvent = eventid;
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

  case 3:
    ntp->GetEvent(fCurrentEvent);
    evetree->GetEvent(fRealEvent);
    //std::cout << fCurrentEvent << " px:py:pz " << fX << " " << fY << " " << fZ << std::endl;
    p.SetXYZ(fX,fY,fZ);
    p *= fP;
    fCurrentEvent++;
    fRealEvent++;

    break;

    default:

      return kFALSE;
  }

  //  p.SetXYZ(0,0,0.2);
  //  fVtxX = fV3Vertex.X() * 10;  
  //  fVtxY = fV3Vertex.Y() * 10;  
  //  fVtxZ = fV3Vertex.Z() * 10;
  std::cout << fCurrentEvent << " " << fRealEvent << " (eveid: " << fTreeEventID << " ) " << fPDG << " " << p.X() << " " << p.Y() << " " << p.Z() << " "
	    << fVtxX/10. << " " << fVtxY/10. << " " << fVtxZ/10. << std::endl;


  primGen->AddTrack(fPDG,p.X(),p.Y(),p.Z(),fVtxX/10.,fVtxY/10.,fVtxZ/10.);

  auto event = (FairMCEventHeader*)primGen->GetEvent();
  if( event && !(event->IsSet()) ){
    event->MarkSet(kTRUE);
    event->SetVertex(TVector3(fVtxX,fVtxY,fVtxZ));
  }

  return kTRUE;
}

