//---------------------------------------------------------------------
// File and Version Information:
// $Id$
//
// Description:
//      STGas reads in gas property file and stores them for later use.
//
// Author List:
//      Genie Jhang     Korea Univ.            (original author)
//      JungWoo Lee     Korea Univ.
//
//----------------------------------------------------------------------

#ifndef _STGAS_H_
#define _STGAS_H_

// FAIRROOT classes
#include "FairParGenericSet.h"
#include "FairParamList.h"
#include "FairLogger.h"

// ROOT classes
#include "TString.h"
#include "TSystem.h"

//#include "Rtypes.h"

class STGas : public FairParGenericSet
{
  public:
    // Constructor and Destructor
    STGas();
    ~STGas();

    void operator=(const STGas& GasToCopy);

    // Getter
    Double_t GetEIonize();
    Double_t GetDriftVelocity();
    Double_t GetCoefAttachment();
    Double_t GetCoefDiffusionLong();
    Double_t GetCoefDiffusionTrans();
    Int_t    GetGain();
    UInt_t   GetRandomCS();

    virtual Bool_t getParams(FairParamList *paramList);
    TString GetFile(Int_t fileNum);

    // Setter
    virtual void putParams(FairParamList *paramList);

  private:
    FairLogger *fLogger;

    Bool_t fInitialized;

    Double_t fEIonize;                  //!< effective ionization energy [eV]
    Double_t fDriftVelocity;            //!< drift velocity [cm/ns]
    Double_t fCoefAttachment;           //!< attachment coefficient
    Double_t fCoefDiffusionLong;        //!< longitudinal diffusion coefficient
    Double_t fCoefDiffusionTrans;       //!< transversal diffusion coefficient
    Double_t fGain;                     //!< gain factor from wire plane

  ClassDef(STGas, 1)
};

#endif
