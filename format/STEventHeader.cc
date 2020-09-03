#include "STEventHeader.hh"

#include <iostream>
using namespace std;

ClassImp(STEventHeader)

STEventHeader::STEventHeader()
{
  Clear();
}

void STEventHeader::Clear(Option_t *option)
{
  fEventID = -1;
  fStatus = -1;
  fAoQ = -1;
  fZ = -1;
  ntrk = -1;
  ngtrk = -1;
  nvtrk = -1;
}

void STEventHeader::Print(Option_t *option) const
{
  TString status = "Not Set Event";

       if (fStatus == 0) status = "Empty Event";
  else if (fStatus == 1) status = "Collision Event";
  else if (fStatus == 2) status = "Active Target Event";
  else if (fStatus == 3) status = "Off Target Event";
  else if (fStatus == 4) status = "Beam Event";
  else if (fStatus == 5) status = "Cosmic Event";
  else if (fStatus == 6) status = "Bad Event";

  cout << " STEventHeader:  Event " << fEventID << "  " << status << endl;
}

Int_t STEventHeader::GetEventID()            { return fEventID; }
void  STEventHeader::SetEventID(Int_t val)   { fEventID = val; }


Int_t STEventHeader::GetStatus()            { return fStatus; }
Bool_t STEventHeader::IsEmptyEvent()        { return fStatus == 0 ? true : false; }
Bool_t STEventHeader::IsCollisionEvent()    { return fStatus == 1 ? true : false; }
Bool_t STEventHeader::IsActiveTargetEvent() { return fStatus == 2 ? true : false; }
Bool_t STEventHeader::IsOffTargetEvent()    { return fStatus == 3 ? true : false; }
Bool_t STEventHeader::IsBeamEvent()         { return fStatus == 4 ? true : false; }
Bool_t STEventHeader::IsCosmicEvent()       { return fStatus == 5 ? true : false; }
Bool_t STEventHeader::IsBadEvent()          { return fStatus == 6 ? true : false; }
Bool_t STEventHeader::IsGGCloseEvent()      { return fStatus == 7 ? true : false; }

void STEventHeader::SetStatus(Int_t val)       { fStatus = val; }
void STEventHeader::SetIsEmptyEvent()          { fStatus = 0; }
void STEventHeader::SetIsCollisionEvent()      { fStatus = 1; }
void STEventHeader::SetIsActiveTargetEvent()   { fStatus = 2; }
void STEventHeader::SetIsOffTargetEvent()      { fStatus = 3; }
void STEventHeader::SetIsBeamEvent()           { fStatus = 4; }
void STEventHeader::SetIsCosmicEvent()         { fStatus = 5; }
void STEventHeader::SetIsBadEvent()            { fStatus = 6; }
void STEventHeader::SetIsGGCloseEvent()        { fStatus = 7; }

void STEventHeader::SetAoQ(Double_t val)       { fAoQ = val; }
void STEventHeader::SetZ(Double_t val)       { fZ = val; }
Double_t STEventHeader::GetAoQ()       { return fAoQ; }
Double_t STEventHeader::GetZ()       { return fZ; }

void STEventHeader::SetNRecoTrk(Int_t val)       { ntrk = val; }
void STEventHeader::SetNGoodRecoTrk(Int_t val)       { ngtrk = val; }
void STEventHeader::SetNVATrk(Int_t val)       { nvtrk = val; }
Int_t STEventHeader::GetNRecoTrk()       { return ntrk; }
Int_t STEventHeader::GetNGoodRecoTrk()       { return ngtrk; }
Int_t STEventHeader::GetNVATrk()       { return nvtrk; }

void STEventHeader::SetTbOffsets(Double_t *tbOffsets)
{
  for (auto coboIdx = 0; coboIdx < 12; ++coboIdx)
    fTbOffsets[coboIdx] = tbOffsets[coboIdx];
}

Double_t *STEventHeader::GetTbOffsets() { return fTbOffsets; }
