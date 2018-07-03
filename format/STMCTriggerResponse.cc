#include "STMCTriggerResponse.hh"
#include "STMCScintillatorHit.hh"

STMCTriggerResponse::STMCTriggerResponse()
: TNamed(),
  fIsTrigger(kFALSE),
  fMult(0), fMultWCT(0),
  fVetoMaxEdep(0.), fVetoMaxZ(0)
{
  fScintillatorHitArray = new TClonesArray("STMCScintillatorHit");
}

STMCTriggerResponse::~STMCTriggerResponse()
{
}

void STMCTriggerResponse::SetScintillatorHitArray(TClonesArray* hitArray)
{
  if( !(hitArray&&hitArray->GetEntries()>0) ) return;

  for(Int_t i=0; i<hitArray->GetEntries(); i++){
    auto hit = (STMCScintillatorHit*) hitArray->At(i);
    new((*fScintillatorHitArray)[i]) STMCScintillatorHit(*hit);
  }
}

ClassImp(STMCTriggerResponse);
