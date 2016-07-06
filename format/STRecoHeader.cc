#include "STRecoHeader.hh"
#include <iostream>
using namespace std;

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
  cout << "    [Name]                    [Value]" << endl;
  for (auto i=0; i<fParList->GetEntries(); i++) {
    TString className = fParList -> At(i) -> ClassName();

    if (className == "TNamed") {
      auto par = (TNamed *) fParList -> At(i);
      auto key = par -> GetName();
      auto value = par -> GetTitle();
      cout << right << setw(2) << i << "  " << left << setw(25) << key << " " << value << endl;
    }
    else if (className == "TParameter<int>") {
      auto par = (TParameter<int> *) fParList -> At(i);
      auto key = par -> GetName();
      int value = par -> GetVal();
      cout << right << setw(2) << i << "  " << left << setw(25) << key << " " << value << endl;
    }
    else if (className == "TParameter<double>") {
      auto par = (TParameter<double> *) fParList -> At(i);
      auto key = par -> GetName();
      double value = par -> GetVal();
      cout << right << setw(2) << i << "  " << left << setw(25) << key << " " << value << endl;
    }
    else if (className == "TParameter<bool>") {
      auto par = (TParameter<bool> *) fParList -> At(i);
      auto key = par -> GetName();
      TString value = par -> GetVal() == true ? "true" : "false";
      cout << right << setw(2) << i << "  " << left << setw(25) << key << " " << value << endl;
    }
  }
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
