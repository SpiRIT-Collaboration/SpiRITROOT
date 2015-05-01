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

// SpiRITROOT classes
#include "STEvent.hh"

// ROOT classes
#include "Rtypes.h"
#include "TVector3.h"
#include "TMatrixD.h"

class STSystemManipulator {
  public:
    STSystemManipulator();
    ~STSystemManipulator();

    STEvent *Change(STEvent *event);
    STEvent *Restore(STEvent *event);

  private:
    void Translate(TVector3 &vector, TVector3 trans);
    void Exchange(TVector3 &vector);
    void Exchange(TMatrixD &matrix);

    TVector3 fTrans;   //!< default translation vector - [mm]

  ClassDef(STSystemManipulator, 1);
};

#endif
