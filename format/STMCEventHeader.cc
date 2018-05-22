#include "STMCEventHeader.hh"
#include "TLorentzVector.h"

STMCEventHeader::STMCEventHeader()
: TNamed(),
  fBeamA(0), fBeamZ(0), fBeamE(0.),
  fTargetA(0), fTargetZ(0)
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
   Double_t amu   = 931.;
   Double_t beamM = amu*fBeamA;
   Double_t beamE = (amu+fBeamE)*fBeamA;
   Double_t beamP = TMath::Sqrt( beamE*beamE - beamM*beamM );
   Double_t targetM = amu*fTargetA;
   TLorentzVector v(0.,0.,beamP,beamE+targetM);
   return v.BoostVector().Z();
}

ClassImp(STMCEventHeader);

