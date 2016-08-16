#include "STMCRecoMatching.hh"

ClassImp(STMCRecoMatching)

STMCRecoMatching::STMCRecoMatching()
: fStatus(STMCRecoMatching::kNotSet), fMCID(-1), fRecoID(-1), fMCPDGCode(0), fMCMomentum(TVector3(0,0,0)), fRecoMomentum(TVector3(0,0,0))
{
}

void STMCRecoMatching::Init(Int_t mcid, Int_t recoid, Int_t mcpdg, TVector3 mcp, TVector3 recop)
{
  fMCID = mcid;
  fRecoID = recoid;
  fMCPDGCode = mcpdg;
  fMCMomentum = mcp;
  fRecoMomentum = recop;
}

void STMCRecoMatching::SetMCID(Int_t val)          { fMCID = val; }
void STMCRecoMatching::SetRecoID(Int_t val)        { fRecoID = val; }
void STMCRecoMatching::SetMCPDGCode(Int_t val)     { fMCPDGCode = val; }
void STMCRecoMatching::SetMCMomentum(TVector3 p)   { fMCMomentum = p; }
void STMCRecoMatching::SetRecoMomentum(TVector3 p) { fRecoMomentum = p; }

void STMCRecoMatching::SetIsMatched()  { fStatus = kMatch; }
void STMCRecoMatching::SetIsNotFound() { fStatus = kNotFound; }
void STMCRecoMatching::SetIsFake()     { fStatus = kFake; }
void STMCRecoMatching::SetStatus(STMatchingStatus val) { fStatus = val; }

Int_t STMCRecoMatching::GetMCID()            { return fMCID; }
Int_t STMCRecoMatching::GetRecoID()          { return fRecoID; }
Int_t STMCRecoMatching::GetMCPDGCode()       { return fMCPDGCode; }
TVector3 STMCRecoMatching::GetMCMomentum()   { return fMCMomentum; }
TVector3 STMCRecoMatching::GetRecoMomentum() { return fRecoMomentum; }

bool STMCRecoMatching::IsMatched()  { return fStatus == kMatch    ? true : false; }
bool STMCRecoMatching::IsNotFound() { return fStatus == kNotFound ? true : false; }
bool STMCRecoMatching::IsFake()     { return fStatus == kFake     ? true : false; }
STMCRecoMatching::STMatchingStatus STMCRecoMatching::GetStatus() { return fStatus; }
