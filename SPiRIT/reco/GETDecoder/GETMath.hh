// =================================================
//  GETMath Class
// 
//  Author:
//    Genie Jhang ( geniejhang@majimak.com )
//  
//  Log:
//    - 2013. 10. 24
//      Start writing class
// =================================================

#ifndef _GETMATH_H_
#define _GETMATH_H_

#include "TObject.h"
#include "TROOT.h"

class GETDecoder;
class GETFrame;

/*!
  * Basically, what this class does is the same as TH1D class in ROOT. 
  * The difference is that this class only provides the **mean** and **RMS** values.
  * Nothing else!
 **/

class GETMath : public TObject
{
  public:
    //! Constructor
    GETMath();
    //! Constructor with GETDecoder class pointer
    GETMath(GETDecoder *decoder);
    //! Destructor
    virtual ~GETMath();

    //! With the mean and RMS, calculated before, use the **value** to calculate new **mean** and **RMS** recursively.
    void Add(Double_t value);
    //! Return the calculated **mean** value.
    Double_t GetMean();
    //! Return the calculated **RMS** value.
    Double_t GetRMS();
    //! Return the calculated **RMS squared** value.
    Double_t GetRMS2();

    /// Set the values manually. Note that the last argument is rms squared, that is variance.
    void Set(Int_t numValues, Double_t mean, Double_t rms2);
    
    //! Temporary
    Double_t **GetAverage(Int_t numChannels, Int_t *chList, Int_t frameNo = -1);

    //! Reset all the values.
    void Reset();

  private:
    GETDecoder *fDecoder; /// GETDecoder class pointer
    GETFrame *fFrame; /// GETFrame class pointer

    Int_t fNumValues; /// Number of values added
    Double_t fMean; /// mean value
    Double_t fRms; /// RMS value

    Double_t *fAdc[4]; /// Average value storage for GetAverage() method
    
  //! Added for dictionary making by ROOT
  ClassDef(GETMath, 1); 
};

#endif
