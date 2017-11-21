/** 
 * STParticle Class
 *
 * @author Mizuki
 */

#include "STParticle.hh"

#include <iostream>

ClassImp(STParticle)

STParticle::STParticle() : ftrackID(-1)
{
  Clear();

}


STParticle::STParticle(const STParticle &cp)
{
  ftrackID       = cp.ftrackID;;
  ftrackatTarget = cp.ftrackatTarget;

  forigP3       = cp.forigP3;
  fRotatedP3    = cp.fRotatedP3;
  fRotatedPt    = cp.fRotatedPt;
  fRotatedP     = cp.fRotatedP;
  fP            = cp.fP;
  fdEdx         = cp.fdEdx;
  ftheta        = cp.ftheta;
  frtheta       = cp.frtheta;
  fphi          = cp.fphi;
  frphi         = cp.frphi;
  frpphi        = cp.frpphi;
  fwgt          = cp.fwgt;
  fPID          = cp.fPID;
  fPIDProbability = cp.fPIDProbability;  

  fRapidity     = cp.fRapidity;
  fpsudoRapidity= cp.fpsudoRapidity;
  fEtotal       = cp.fEtotal;
  fChar         = cp.fChar;
  fMass         = cp.fMass;

  fpipid        = cp.fpipid;
  fvertex       = cp.fvertex;

  //flags
  fBeamonTargetf   = cp.fBeamonTargetf;
  fVatTargetf      = cp.fVatTargetf;
  fVZatTargetf     = cp.fVZatTargetf;
  fVBDCCorf        = cp.fVBDCCorf;
  fBDCCorf         = cp.fBDCCorf;
  fTargetXYf       = cp.fTargetXYf;
  fgotoKatanaf     = cp.fgotoKatanaf;
  fgotoKyotof      = cp.fgotoKyotof;
  frdEdxPointSizef = cp.frdEdxPointSizef;
  fgoodtrackf      = cp.fgoodtrackf;
  fReactionPlanef  = cp.fReactionPlanef;

  //mixed event
  fmxevt           = cp.fmxevt;
  fmxntrk          = cp.fmxntrk;
  fmxtrackid       = cp.fmxtrackid;


  //STRecoTrack parameters
  fRTrack          = cp.fRTrack;
  rVertexID        = cp.rVertexID;  
  rdEdxPointSize   = cp.rdEdxPointSize;
  rdEdxPointSize_thr = cp.rdEdxPointSize_thr;
  rNDF             = cp.rNDF;
  rDist            = cp.rDist;
  rpocaVertex      = cp.rpocaVertex;

  //  std::cout << "initi " << fPID << " : " << cp.fPID << endl;
}


STParticle &STParticle::operator=(const STParticle &cp)
{

  if( this != &cp )
    *this = cp;

  return *this;
}


void STParticle::CheckTrackonTarget()
{
  // Track XY
  Double_t trktgt_right =  -10.2; //!
  Double_t trktgt_left  =   16.2; //!
  Double_t trktgt_top   = -210.; //!
  Double_t trktgt_btm   = -235.; //!


  if(ftrackatTarget.X() >= trktgt_right && ftrackatTarget.X() <= trktgt_left &&
     ftrackatTarget.Y() >= trktgt_btm   && ftrackatTarget.Y() <= trktgt_top)
    fTargetXYf = 1;
  else
    fTargetXYf = 0;
}


void STParticle::Clear(Option_t *option)
{

  fRotatedP3 = TVector3(-9999,-9999,-9999);
  forigP3    = TVector3(-9999,-9999,-9999);
  fvertex    = TVector3(-9999,-9999,-9999);

  fP            = -9999.;
  fdEdx         = -9999.;
  ftheta        = -10.;
  frtheta       = -10.;
  fphi          = -10.;
  frphi         = -10.;


  fpipid       = 0;
  fPID         = 0;


  // for flow
  ffltnP3 = TVector3(-9999,-9999,-9999);
  ffltnPt = TVector2(-9999,-9999);

  frpphi = -10.;
  fwgt   = 0.;
  fcorrBin[0] = -1;  
  fcorrBin[1] = -1;
  
  fmxevt = -1;
  fmxntrk = -1;

  fcorrPt = ffltnPt;
}


void STParticle::SetRecoTrack(STRecoTrack *atrack)
{
  fRTrack = atrack;

  forigP3 = fRTrack->GetMomentumTargetPlane();

  //  forigP3 = fRTrack->GetMomentum();  // modified on 9 Nov. 2017
  //Because of PZ bug for v1.04
  if(forigP3.Z() < 0)
    forigP3 = -forigP3;

  fphi  = forigP3.Phi();
  ftheta= forigP3.Theta();
  fP    = forigP3.Mag();
  fdEdx = fRTrack->GetdEdxWithCut(0, 0.7);
  fChar = fRTrack->GetCharge();

  fPID  = STPID::GetPDG(fRTrack->GetPID());
  fPIDProbability = fRTrack->GetPIDProbability();

  fRotatedP3 = forigP3;

  rVertexID      =  fRTrack -> GetVertexID();
  rdEdxPointSize = (fRTrack -> GetdEdxPointArray()) -> size();
  rNDF           =  fRTrack -> GetClusterIDArray()  -> size();
  rpocaVertex    =  fRTrack -> GetPOCAVertex();
 
  SetMass();
  SetProperty();
}




void STParticle::SetProperty()
{
  
  //  SetLinearPID();
  //  CheckTrackonTarget();
  CheckKATANAHit();
  CheckKYOTOHit();

  SetPiPID();

  
}

void STParticle::SetTrackAtTarget(TVector3 value)  
{
  ftrackatTarget = value; 

  rDist = (rpocaVertex - ftrackatTarget).Mag();


  //  CheckTrackonTarget();
}

void STParticle::SetPID(Int_t value)
{
  fPID = value;

  SetMass();

  SetRapidity();
}

void STParticle::SetLinearPID()
{
  //  for run2900
  Double_t alpha = 2.3;
  //

  flnPIDval = log(alpha*fdEdx)+(alpha-1)*log(fP+0.5*fdEdx);

  flnPID = 0;
  if(fP > 100){
    if(flnPIDval <= 12.){
      if(fChar > 0)
	flnPID = 211;
      else
	flnPID = -211;
    }
    else if(flnPIDval <= 14.3 || fP <= 430) { // proton
      flnPID = 2212;
      fMass  = 938.272;
    }
    else if(flnPIDval <=15.1) { // deuteron
      flnPID = 10020;
      fMass  = 1875.6128;
    }
    else if(flnPIDval <=15.4) { // trition
      flnPID = 10030;
      fMass  = 2808.920936;
    }
    else if(flnPIDval <=15.8) { // 3He
      flnPID = 20030;
      fMass  = 2808.391346;
    }
    else if(flnPIDval <=16.3)  { // 4He
      flnPID = 20040;
      fMass  = 3727.37909;
    }
    else {
      flnPID = 30000;
      fMass  = 0.;
    }
  }
  else {
    if(fChar > 0)
      flnPID = 211;
    else
      flnPID = -211;
  }

  fPID = flnPID;

}


void STParticle::SetMass()
{
  auto mpi  =    139.57018;
  auto mp   =    938.2720813;
  auto mn   =    939.565346;
  auto md   =   1875.612762;
  auto mt   =   2808.921112;
  auto mhe3 =   2808.39132;
  auto mal  =   3727.379378;


  Double_t mass = 0;
  switch (fPID) {
  case 12212:
  case 2212:
    mass = mp;
    break;

  case 211:
  case -211:
    mass = mpi;
    break;

  case 1000010020:
    mass = md;
    break;

  case 1000010030:
    mass = mt;
    break;

  case 1000020030:
    fChar = 2;
    mass = mhe3;
    break;

  case 1000020040:
    fChar = 2;
    mass = mal;
    break;

  default:
    mass = 0;
    break;
  }

  fMass = mass;

}


void STParticle::SetpsudoRapidity()
{
  fpsudoRapidity = -log( tan((fRotatedP3.Theta()/2)) );
}

 
void STParticle::SetRapidity()
{
  Double_t P    = fRotatedP3.Mag();
  Double_t Pz   = fRotatedP3.Z();


  if(fMass != 0 ){
    fEtotal   = sqrt(fMass*fMass + P*P);
    fRapidity = 0.5 * log( (fEtotal + Pz) / (fEtotal - Pz) );
  }
  else {
    fEtotal = 0;
    fRapidity = -10.;
  }

  SetpsudoRapidity();
}


void  STParticle::RotateAlongBeamDirection(Double_t valuex, Double_t valuey)
{

  fRotatedP3.RotateY(-valuex);
  fRotatedP3.RotateX(-valuey);

  fRotatedPt = TVector2(fRotatedP3.X(),fRotatedP3.Y());

  fcorrPt = fRotatedPt;

  frphi   = fRotatedP3.Phi();
  frtheta = fRotatedP3.Theta();

  bRotated = kTRUE;
}

void STParticle::Flattening(Double_t value)
{
  ffltnP3  = fRotatedP3;

  ffltnP3.SetPhi(value);

  ffltnPt  = TVector2(ffltnP3.X(), ffltnP3.Y());

  fcorrPt  = ffltnPt;

  frphi    = ffltnP3.Phi();

  frtheta  = ffltnP3.Theta();

  bFlatten = kTRUE;
}


void STParticle::SetPiPID()
{
  //pion cut                                                                                                                                               
  // TFile *gcutPiFile = new TFile("/cache/scr/spirit/mizuki/SpiRITROOT/macros/AnalysisMacro/gcutPip.root");
  // TCutG *gPip = (TCutG*)gcutPiFile->Get("gPi");
  // gcutPiFile->Close();

  // if(gPip->IsInside(fdEdx,fP))
  //   fpipid = 1;
  // else
  //   fpipid = 0;

}
