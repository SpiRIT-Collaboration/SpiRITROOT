//-----------------------------------------------------------
// Description:
//   Translate the system for the vertex to be origin
//   and rotate it for the track to make circle on xy plane
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef STSYSTEMMANIPULATOR_H
#define STSYSTEMMANIPULATOR_H

// ROOT classes
#include "Rtypes.h"
#include "TVector3.h"
#include "TMath.h"

class STSystemManipulator {
  public:
    STSystemManipulator();
    ~STSystemManipulator();

    void Change(TVector3 &vector);
    void Restore(TVector3 &vector);

  private:
    void Translate(TVector3 &vector, TVector3 trans);
    void Rotate(TVector3 &vector, Int_t axis, Double_t angle);

    TVector3 fTrans;   //!< default translation vector - [mm]
    Int_t fAxis;       //!< default rotation axis - 0: x, 1: y, 2: z
    Double_t fAngle;   //!< default rotation angle about the default axis - [Rad]
};

#endif
