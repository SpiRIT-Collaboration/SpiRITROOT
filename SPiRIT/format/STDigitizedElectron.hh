//---------------------------------------------------------------------
// Description:
//      Digitized Electron data class header
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

#ifndef _STDIGITIZEDELECTRON_H_
#define _STDIGITIZEDELECTRON_H_

// ROOT class headers
#include "TObject.h"

class STDigitizedElectron : public TObject
{
  public :

    /** Default constructor **/
    STDigitizedElectron();

    STDigitizedElectron(Double_t x,
                        Double_t z,
                        Double_t zWire,
                        Double_t time,
                        Int_t    gain);

    /** Default destructor **/
    ~STDigitizedElectron();

    //Getters
    Double_t GetX();
    Double_t GetZ();
    Double_t GetZWire();
    Double_t GetTime();
    Int_t    GetGain();

    //Setters
    void SetIndex(Int_t index);
    
  private :
    Double_t fX;     /// x position [cm]
    Double_t fZ;     /// z position [cm]
    Double_t fZWire; /// z position of wire where electron is absorbed[cm]
    Double_t fTime;  /// arrival time on wire plane [ns]
    Int_t    fGain;  /// amound of gain in wire plane

    Int_t    fIndex; /// position of STDigitizedElectron in "TClonesArray"

  ClassDef(STDigitizedElectron, 1)
};

#endif
