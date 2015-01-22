#pragma once 

#include "TObject.h"

class STDriftedElectron : public TObject
{
  public :

    /** Default constructor **/
    STDriftedElectron();

    /** Constructor with input **/
    STDriftedElectron(Double_t x,
                      Double_t dx,
                      Double_t z,
                      Double_t dz,
                      Double_t t,
                      Double_t dt,
                      Int_t    iWire,
                      Int_t    zWire,
                      Int_t    gain);

    /** Default destructor **/
    ~STDriftedElectron();

    //Getters
    Double_t GetX();
    Double_t GetDX();
    Double_t GetZ();
    Double_t GetDZ();
    Double_t GetT();
    Double_t GetDT();
    Int_t    GetIWire();
    Int_t    GetZWire();
    Int_t    GetGain();

    //Setters
    void SetIndex(Int_t index);
    
  private :
    Double_t fX;     ///< x position [mm]
    Double_t fDX;    ///< x diffusion [mm]
    Double_t fZ;     ///< z position [mm]
    Double_t fDZ;    ///< z diffusion [mm]
    Double_t fT;     ///< arrival time on wire plane [ns]
    Double_t fDT;    ///< diffused time [ns]
    Int_t    fIWire; ///< numbering of wire where electron is absorbed (negative value is posible)
    Int_t    fZWire; ///< z position of wire where electron is absorbed [mm]
    Int_t    fGain;  ///< amount of gain in wire plane

    Int_t    fIndex; /// position of STDriftedElectron in "TClonesArray"

  ClassDef(STDriftedElectron, 1)
};
