/**
 * @brief Calculate z-position of wire
 *
 * @author JungWoo Lee (Korea Univ.)
 */

#pragma once 

#include "STDigiPar.hh"
#include "STGas.hh"
#include "TLorentzVector.h"

class STWireResponse
{
  public :

    STWireResponse();
    ~STWireResponse() {};

    /**
     * Recieve z-position of electron in anode wire plane, calculate closest
     * wire, return z-position of wire.
     */
    Int_t FindZWire(Double_t z);

  private :

    STDigiPar* fPar;

    Int_t fZCenterWire;   //!< z-position of center wire [mm]
    Int_t fZSpacingWire;  //!< Spacing value of wires [mm]
    Int_t fZFirstWire;    //!< z-position of first wire (minimum z) [mm]
    Int_t fZLastWire;     //!< z-position of last wire (maximum z) [mm]

  ClassDef(STWireResponse, 1);
};
