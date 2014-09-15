//---------------------------------------------------------------------
// Description:
//      Avalanche data class source
//
//      Data class for drifted elctron STAvalacheTask.
//
//      Created by : STAvalancheTask
//      Used    by : STPadResponseTask
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

#ifndef _STAVALANCHE_H_
#define _STAVALANCHE_H_

// ROOT class headers
#include "TObject.h"

class STAvalanche : public TObject
{
  public :

    /** Default constructor **/
    STAvalanche();

    STAvalanche(Double_t x,
                Double_t z,
                Double_t time,
                Double_t gain);

    /** Default destructor **/
    ~STAvalanche();

    //Getters
    Double_t GetX();
    Double_t GetZ();
    Double_t GetTime();
    Double_t GetGain();

    //Setters
    void SetIndex(Int_t index);
    
  private :
    Double_t fX;     ///  x position [cm]
    Double_t fZ;     ///  z position [cm]
    Double_t fTime;  ///  arrival time on wire plane from primary collision time[ns]
    Double_t fGain;  ///  amplification factor of electron

    Int_t    fIndex; ///  position of STAvalanche in "TClonesArray"

  ClassDef(STAvalanche,1)
};

#endif
