//---------------------------------------------------------------------
// Description:
//      Drifted Electron data class header
//
//      Data class for drifted elctron STDriftTask.
//      Drifted electron is electrons drifted through gas
//      with diffusions
//
//      Created by : STDriftTask
//      Used    by : STAvalacheTask
//
// Author List:
//      JungWoo Lee     Korea Univ.       (original author)
//
//----------------------------------------------------------------------

#ifndef _STDRIFTEDELECTRON_H_
#define _STDRIFTEDELECTRON_H_

// ROOT class headers
#include "TObject.h"

class STDriftedElectron : public TObject
{
  public :

    /** Default constructor **/
    STDriftedElectron();

    STDriftedElectron(Double_t x,
                      Double_t z,
                      Double_t time);

    /** Default destructor **/
    ~STDriftedElectron();

    //Getters
    Double_t GetX();
    Double_t GetZ();
    Double_t GetTime();

    //Setters
    void SetIndex(Int_t index);
    
  private :
    Double_t fX;     //!<  x position [cm]
    Double_t fZ;     //!<  z position [cm]
    Double_t fTime;  //!<  arrival time on wire plane from primary collision time[ns]

    Int_t    fIndex; //!<  position of STDriftedElectron in "TClonesArray"

  ClassDef(STDriftedElectron,1)
};

#endif
