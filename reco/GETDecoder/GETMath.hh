// =================================================
//  GETMath Class
// 
//  Author:
//    Genie Jhang ( geniejhang@majimak.com )
//  
//  Log:
//    - 2013. 10. 24
//      Start writing class
//    - 2015. 11. 13
//      Adopted to the new version
// =================================================

#ifndef GETMATH
#define GETMATH

#include "Rtypes.h"

/**
  * Basically, what this class does is the same as TH1D class in ROOT. 
  * The difference is that this class only provides the **mean** and **RMS** values.
  * Nothing else!
 **/

class GETMath
{
  public:
    GETMath();

        void Reset();             ///< Reset all the values.

        void Add(Double_t value); ///< With the mean and RMS, calculated before, use the **value** to calculate new **mean** and **RMS** recursively.
    Double_t GetMean();           ///< Return the calculated **mean** value.
    Double_t GetRMS();            ///< Return the calculated **RMS** value.
    Double_t GetRMS2();           ///< Return the calculated **RMS squared** value.

        void Set(Int_t numValues, Double_t mean, Double_t rms2);  /// Set the values manually. Note that the last argument is rms squared, that is variance.
    

  private:
       Int_t fNumValues; ///< Number of values added
    Double_t fMean;      ///< mean value
    Double_t fRms;       ///< RMS value

  ClassDef(GETMath, 1)   ///< Added for dictionary making by ROOT
};

#endif
