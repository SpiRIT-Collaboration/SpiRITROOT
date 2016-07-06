#ifndef STRUNHEADER
#define STRUNHEADER

#include "FairFileHeader.h"

#include "TList.h"
#include "TNamed.h"
#include "TParameter.h"

class STRecoHeader : public TNamed
{
  public:
    STRecoHeader();
    STRecoHeader(const char *name, const char *title);
    virtual ~STRecoHeader() {}

    virtual void Print(Option_t *option="") const;

    void SetPar(TString name, Bool_t val);
    void SetPar(TString name, Int_t val);
    void SetPar(TString name, Double_t val);
    void SetPar(TString name, TString val);

    TList *GetParList();

      Bool_t GetParBool(TString name);
       Int_t GetParInt(TString name);
    Double_t GetParDouble(TString name);
     TString GetParString(TString name);

  protected:
    TList* fParList;

  ClassDef(STRecoHeader, 1)
};

#endif
