#include "STMCEventHeader.hh"
#include "TLorentzVector.h"

STMCEventHeader::STMCEventHeader()
:TNamed("STMCEventHeader","Event Header for MC"),
  fBeamVector(TLorentzVector()),
  fTargetVector(TLorentzVector())
{
  Clear();
  fPrimaryTrackArray = new TClonesArray("STMCTrack");
}


STMCEventHeader::~STMCEventHeader()
{}

void STMCEventHeader::Clear(Option_t* option)
{
  fB = -1.;
  fPrimaryVertex = TVector3();
  fBeamAngle = TVector2();
  fReactionPlane = -999.;
}

void STMCEventHeader::SetPrimaryTracks(TClonesArray* input)
{
  if( !(input&&input->GetEntries()>0) ) return;

  if(fPrimaryTrackArray)
    fPrimaryTrackArray->Clear();

  for(Int_t i=0; i<input->GetEntries(); i++){
    STMCTrack* inTrack = (STMCTrack*)input->At(i);
    new((*fPrimaryTrackArray)[i]) STMCTrack(*inTrack);
  }

}


Double_t STMCEventHeader::GetLorentzTransformFactor()
{
  TLorentzVector v(fBeamVector.Vect()+fTargetVector.Vect(),fBeamVector.E()+fTargetVector.E());
  return v.BoostVector().Z();
}

TRotation STMCEventHeader::GetRotationInRotatedFrame()
{
  Double_t beamAngA = fBeamAngle.X();
  Double_t beamAngB = fBeamAngle.Y();
  Double_t tanA = TMath::Tan(beamAngA);
  Double_t tanB = TMath::Tan(beamAngB);

  TRotation rotatedFrame;
  rotatedFrame.RotateY(beamAngA);
  TVector3 axisX(1,0,0);
  axisX.Transform(rotatedFrame);
  Double_t thetaPrime = TMath::ACos(TMath::Sqrt( (1.+tanA*tanA)/(tanA*tanA+tanB*tanB+1.) ));
  TRotation rotateInRotatedFrame;
  rotateInRotatedFrame.Rotate(thetaPrime,axisX);

  return rotateInRotatedFrame;
}

TClonesArray* STMCEventHeader::GetPrimaryTracksWithRotation()
{
  if(!fPrimaryTrackArray) return nullptr;

  Double_t beamAngA = fBeamAngle.X();
  TRotation rotateInRotatedFrame = GetRotationInRotatedFrame();

  TClonesArray* outArray = new TClonesArray("STMCTrack");

  for(Int_t i=0; i<fPrimaryTrackArray->GetEntries(); i++){
    STMCTrack* primary = (STMCTrack*) fPrimaryTrackArray->At(i);
    TLorentzVector v;
    primary->Get4Momentum(v);
    v.Transform(rotateInRotatedFrame);
    v.RotateY(-beamAngA);
    /* end reversing the rotation of beam -> only reaction plane rotation remains.*/
    new((*outArray)[i]) STMCTrack(*primary);
    STMCTrack* copy = (STMCTrack*)outArray->At(i);
    copy->SetMomentum(v.Vect());
  }

  return outArray;

}

STMCTrack* STMCEventHeader::GetPrimaryTrackWithRotation(Int_t id)
{
  if(!fPrimaryTrackArray) return nullptr;
  if(id>=fPrimaryTrackArray->GetEntries()) return nullptr;

  Double_t beamAngA = fBeamAngle.X();
  TRotation rotateInRotatedFrame = GetRotationInRotatedFrame();

  STMCTrack* primary = (STMCTrack*) fPrimaryTrackArray->At(id);
  TLorentzVector v;
  primary->Get4Momentum(v);
  v.Transform(rotateInRotatedFrame);
  v.RotateY(-beamAngA);
  /* end reversing the rotation of beam -> only reaction plane rotation remains.*/
  STMCTrack* copy = new STMCTrack(*primary);
  copy->SetMomentum(v.Vect());

  return copy;

}

ClassImp(STMCEventHeader);
