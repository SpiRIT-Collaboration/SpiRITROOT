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

#include "Rtypes.h"

class STGas
{
  public:
    // Constructor and Destructor
    STGas();
    ~STGas();

    void operator=(const STGas& GasToCopy);

    // Getter
    Double_t GetEIonize();
    UInt_t   GetRandomCS();

    Double_t GetDriftVelocity();
    Double_t GetCoefAttachment();
    Double_t GetCoefDiffusion();

    // Setter

  private:
    Double_t fEIonize;                  //!< effective ionization energy [eV]
    Double_t fDriftVelocity;            //!< drift velocity [cm/ns]
    Double_t fCoefAttachment;           //!< attachment coefficient
    Double_t fCoefDiffusion;            //!< diffusion coefficient

  ClassDef(STGas, 1)
};

#endif
