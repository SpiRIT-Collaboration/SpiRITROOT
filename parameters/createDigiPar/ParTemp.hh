#ifndef STDIGIPAR_HH
#define STDIGIPAR_HH

// FAIRROOT classes
#include "FairParGenericSet.h"
#include "FairParamList.h"
#include "FairLogger.h"

// ROOT classes
#include "TString.h"
#include "TSystem.h"

class STDigiPar : public FairParGenericSet
{
  public :
    STDigiPar(const Char_t *name, const Char_t *title, const Char_t *context);
    ~STDigiPar();

    virtual Bool_t getParams(FairParamList *paramList);
    virtual void putParams(FairParamList *paramList);
    TString GetFile(Int_t fileNum);

    // USER-PUBLIC

    /// returns the time duration of a time bucket in given sampling time in ns.
       Int_t GetTBTime();

  private :
    FairLogger *fLogger;
    Bool_t fInitialized;

    // USER-PRIVATE

  ClassDef(STDigiPar, 1);
};

#endif
