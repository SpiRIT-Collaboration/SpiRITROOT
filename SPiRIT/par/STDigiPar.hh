//---------------------------------------------------------------------
// Description:
//      STDigiPar reads in parameters and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//----------------------------------------------------------------------

#ifndef _STDIGIPAR_H_
#define _STDIGIPAR_H_

// FAIRROOT classes
#include "FairParGenericSet.h"
#include "FairParamList.h"
#include "FairLogger.h"

// SPiRITROOT classes
#include "STGas.hh"

// ROOT classes
#include "TString.h"
#include "TSystem.h"

class STDigiPar : public FairParGenericSet
{
  public:
    // Constructors and Destructors
    STDigiPar(const Char_t *name, const Char_t *title, const Char_t *context);
    ~STDigiPar();

    // Operators
    
    // Getters
    STGas *GetGas();
    virtual Bool_t getParams(FairParamList *paramList);

    TString GetFile(Int_t fileNum);

    // Setters
    virtual void putParams(FairParamList *paramList);

    // Main methods

  private:
    FairLogger *fLogger;

    STGas *fGas;

    Bool_t fInitialized;

    Int_t fGasFile;
    Double_t fEField;
    Int_t fPadPlaneFile;
    Int_t fPadShapeFile;

  ClassDef(STDigiPar, 1);
};

#endif
