#include "STRecoHeader.hh"

ClassImp(STRecoHeader)

STRecoHeader::STRecoHeader()
: fParList(new TList) 
{
  fParList -> SetName("reco_parameters");
}

STRecoHeader::STRecoHeader(const char *name, const char *title)
: TNamed(name, title), fParList(new TList) 
{
  fParList -> SetName("reco_parameters");
}

void STRecoHeader::Print(Option_t *option) const
{
  fParList -> Print();
}

void STRecoHeader::SetPar(TString name, Bool_t val)   { fParList -> Add(new TParameter<Bool_t>(name, val)); }
void STRecoHeader::SetPar(TString name, Int_t val)    { fParList -> Add(new TParameter<Int_t>(name, val)); }
void STRecoHeader::SetPar(TString name, Double_t val) { fParList -> Add(new TParameter<Double_t>(name, val)); }
void STRecoHeader::SetPar(TString name, TString val)  { fParList -> Add(new TNamed(name, val)); }

TList *STRecoHeader::GetParList() { return fParList; }

Bool_t   STRecoHeader::GetParBool(TString name)   { return ((TParameter<Bool_t> *) fParList -> FindObject(name)) -> GetVal(); }
Int_t    STRecoHeader::GetParInt(TString name)    { return ((TParameter<Int_t> *) fParList -> FindObject(name)) -> GetVal(); }
Double_t STRecoHeader::GetParDouble(TString name) { return ((TParameter<Double_t> *) fParList -> FindObject(name)) -> GetVal(); }
TString  STRecoHeader::GetParString(TString name) { return ((TNamed *) fParList -> FindObject(name)) -> GetTitle(); }
