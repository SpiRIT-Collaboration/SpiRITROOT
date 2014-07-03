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

#include "TObject.h"

class STGas
{
  public:
    // Constructor and Destructor
    STGas();
    ~STGas();

    // Getter
    double GetEIonize()                  { return fEIonize };
    double GetFirstIonizationPotential() { return fFirstIonizationPotential };
    double GetRandomCS(double val);

    // Setter

  private:
    
    double fEIonize;                  //!< effective ionization energy [eV]
    double fFirstIonizationPotential; //!< effective ionization energy [eV]

  ClassDef(STGas, 1)
};

#endif
