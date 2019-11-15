/** 111
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
  Initialize();
}


STParticle::STParticle(const STParticle &cp)
{
  bRotated      = cp.bRotated;
  bFlatten      = cp.bFlatten;
  
  ftrackID      = cp.ftrackID;;
  fPID          = cp.fPID;
  fPID_tight    = cp.fPID_tight;
  fPID_norm     = cp.fPID_norm;
  fPID_loose    = cp.fPID_loose;
  fRapidity     = cp.fRapidity;
  fRapiditycm   = cp.fRapiditycm;
  fEtotal       = cp.fEtotal;
  fChar         = cp.fChar;
  fGFChar       = cp.fGFChar;
  fMass         = cp.fMass;
  fBBMass       = cp.fBBMass;
  fBBMassHe     = cp.fBBMassHe;
  fpipid        = cp.fpipid;
  fvertex       = cp.fvertex;
  fPIDProbability = cp.fPIDProbability;  
  fNDF          = cp.fNDF;
  fLzvec        = cp.fLzvec;

  forigP3       = cp.forigP3;
  fP            = cp.fP;
  fdEdx         = cp.fdEdx;

  fRotatedP3    = cp.fRotatedP3;
  fRotatedPt    = cp.fRotatedPt;
  fPxz          = cp.fPxz;
  fPyz          = cp.fPyz;

  frpv          = cp.frpv;
  frpphi        = cp.frpphi;
  fdeltphi      = cp.fdeltphi;
  fwgt          = cp.fwgt;

  //flags
  fBeamonTargetf   = cp.fBeamonTargetf;
  fVBDCCorf        = cp.fVBDCCorf;
  fBDCCorf         = cp.fBDCCorf;
  fTargetf         = cp.fTargetf;
  fgotoKatanaf     = cp.fgotoKatanaf;
  fgotoKyotof      = cp.fgotoKyotof;
  frdEdxPointSizef = cp.frdEdxPointSizef;
  fclusterratiof   = cp.fclusterratiof;
  fVatTargetf      = cp.fVatTargetf;
  fdistanceatvertexf = cp.fdistanceatvertexf;
  fNDFf            = cp.fNDFf;
  fmomentumf       = cp.fmomentumf;
  fdedxf           = cp.fdedxf;

  fmassf           = cp.fmassf;
  
  fgoodtrackf      = cp.fgoodtrackf;
  fReactionPlanef  = cp.fReactionPlanef;

  
  //mixed event
  fmxevt           = cp.fmxevt;
  fmxntrk          = cp.fmxntrk;
  fmxtrackid       = cp.fmxtrackid;


  //STRecoTrack parameters
  fRTrack          = cp.fRTrack;
  fVATrack         = cp.fVATrack;
  rVertexID        = cp.rVertexID;  
  rdEdxPointSize   = cp.rdEdxPointSize;
  rdEdxPointSize_thr = cp.rdEdxPointSize_thr;
  rNDF             = cp.rNDF;
  rDist            = cp.rDist;
  rPOCAVertex      = cp.rPOCAVertex;
  rChi2            = cp.rChi2;
  rClusterSize     = cp.rClusterSize;    
  fclustex         = cp.fclustex;
  fclusterratio    = cp.fclusterratio;

  Initialize();
}

STParticle &STParticle::operator=(const STParticle &cp)
{

  if( this != &cp )
    *this = cp;

  return *this;
}


void STParticle::Initialize()
{
}

void STParticle::Clear(Option_t *option)
{

  fRotatedP3 = TVector3(-9999,-9999,-9999);
  forigP3    = TVector3(-9999,-9999,-9999);
  fvertex    = TVector3(-9999,-9999,-9999);
  fLzvec     = TLorentzVector(0.,0.,0.,0.);
  
  fRotatedPt = TVector2(0.,0.);
  fPxz       = TVector2(0.,0.);
  fPyz       = TVector2(0.,0.);

  fP            = -9999.;
  fdEdx         = -9999.;

  fpipid       = 0;
  fPID         = 0;
  fPID_tight   = 0;
  fPID_norm    = 0;
  fPID_loose   = 0;
  fNDF         = 0.;
  fclustex     = -1.;
  fclusterratio= -1.;

  // Track quality flag
  fBeamonTargetf = 1;
  fVBDCCorf      = 1;
  fBDCCorf       = 1;
  fgoodtrackf  = 1;
  fVatTargetf  = 1;   
  fdistanceatvertexf = 1;
  fTargetf     = 1;
  fmomentumf   = 1;    
  fdedxf       = 1;
  fRapidity    = -9.;
  fRapiditycm  = -9.;

  fNDFf        = 1;    
  fclusterratiof = 1;  
  fmassf       = 1;

  // for flow
  frpv     = TVector3(-9999,-9999,-9999);
  frpphi   = -10.;
  fdeltphi = -10.;
  fwgt   = 0.;
  
  fmxevt = -1;
  fmxntrk = -1;


  fReactionPlanef = 0;

  rChi2   = 0.;
  fBBMass = 0.;
  rClusterSize = 0;

}


void STParticle::SetRecoTrack(STRecoTrack *atrack)
{
  fRTrack = atrack;

  forigP3 = fRTrack->GetMomentumTargetPlane(); //v35
  // forigP3 = fRTrack->GetMomentum();          // v36
  fRotatedP3 = forigP3;

  fRotatedPt = TVector2( fRotatedP3.X(), fRotatedP3.Y());
  fPxz       = TVector2( fRotatedP3.Z(), fRotatedP3.X());
  fPyz       = TVector2( fRotatedP3.Z(), fRotatedP3.Y());

  fP    = forigP3.Mag();
  fdEdx = fRTrack->GetdEdxWithCut(0, 0.7);
  fChar = fRTrack->GetCharge();
  fGFChar = fRTrack->GetGenfitCharge();

  fPID  = STPID::GetPDG(fRTrack->GetPID());
  fPIDProbability = fRTrack->GetPIDProbability();

  rVertexID      =  fRTrack -> GetVertexID();
  rdEdxPointSize = (fRTrack -> GetdEdxPointArray()) -> size();
  rNDF           =  fRTrack -> GetNDF();
  rPOCAVertex    =  fRTrack -> GetPOCAVertex();
  rChi2          =  fRTrack -> GetChi2();
  rClusterSize   = (fRTrack -> GetClusterIDArray()) -> size();

  // quality flag
  if( fP > maxMomentum ) fmomentumf = 0;
  if( fdEdx > maxdEdx  ) fdedxf     = 0;

}

void STParticle::SetVATrack(STGenfitVATask *atrack)
{
  fVATrack = atrack;

  SetRecoTrack( (STRecoTrack*)fVATrack );

  std::cout << "SetVATrack " << std::endl;
}




void STParticle::SetProperty()
{
  
  //  SetLinearPID();
  //  CheckTrackonTarget();
  CheckKATANAHit();
  CheckKYOTOHit();

  SetPiPID();

  
}

void STParticle::SetVertex(STVertex *value)  
{
  fNDF = value->GetNDF();
  SetVertex( value->GetPos() );
}

void STParticle::SetVertex(TVector3 value)  
{
  fvertex = value;
  rDist = (rPOCAVertex - fvertex).Mag();
}

void STParticle::SetPIDLoose(Int_t value)
{
  fPID_loose = value;
  SetMass(value);
}



void STParticle::SetMass(Int_t val)
{
  auto mpi  =    139.57018;
  auto mp   =    938.2720813;
  auto mn   =    939.565346;
  auto md   =   1875.612762;
  auto mt   =   2808.921112;
  auto mhe3 =   2808.39132;
  auto mhe4 =   3727.379378;


  Double_t mass = 0;
  switch (val) {
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
    mass = mhe3;
    fRotatedP3.SetMag(fP * 2.);
    break;

  case 1000020040:
    fRotatedP3.SetMag(fP * 2.);
    mass = mhe4;
    break;

  default:
    mass = 0;
    break;
  }

  fMass = mass;

  SetLorentzVector();
}

void  STParticle::SetLorentzVector()
{
  fEtotal     = sqrt(fMass*fMass + fRotatedP3.Mag2());

  fLzvec.SetVect(fRotatedP3);
  fLzvec.SetT(fEtotal);

  fRapidity   = fLzvec.Rapidity();
    
  if(fMass == 0 ) {
    fEtotal = 0;
    fRapidity = -10.;
    fLzvec.SetT(0.);
  }
}

void  STParticle::RotateAlongBeamDirection(Double_t valuex, Double_t valuey)
{
 
  fRotatedP3.RotateY(-valuex);
  fRotatedP3.RotateX(-valuey);

  SetRotatedPt();

  bRotated = kTRUE;
}

void STParticle::SetRotatedPt()
{
  fRotatedPt = TVector2( fRotatedP3.X(), fRotatedP3.Y());
  fPxz       = TVector2( fRotatedP3.Z(), fRotatedP3.X());
  fPyz       = TVector2( fRotatedP3.Z(), fRotatedP3.Y());  
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
