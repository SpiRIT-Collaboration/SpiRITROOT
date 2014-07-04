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

    // Getter
    Double_t GetEIonize();
    Double_t GetFirstIonizationPotential();
    Double_t GetRandomCS(Double_t val);

    // Setter

  private:
    Double_t fEIonize;                  //!< effective ionization energy [eV]
    Double_t fFirstIonizationPotential; //!< first ionization energy [eV]

  ClassDef(STGas, 1)
};

#endif
