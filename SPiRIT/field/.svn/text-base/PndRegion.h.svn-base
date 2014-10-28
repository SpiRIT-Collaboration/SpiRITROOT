// -------------------------------------------------------------------------
// -----                      PndRegion header file                  -----
// -----                 Created 29/01/07  by M. Al/Turany               -----
// -------------------------------------------------------------------------
/** PndRegion.h
 ** @author M.Al/Turany <m.al-turany@gsi.de>
  ** @since 29.01.2007
 ** @version1.0
 **/

#ifndef PNDREGION_H
#define PNDREGION_H 1
#include "TObject.h"

class PndRegion : public TObject {

public:

  /** Default constructor **/
  PndRegion( Double_t Zmin, Double_t Zmax);
  /** Destructor **/
  virtual ~PndRegion();
  Bool_t IsInside(Double_t Z);
  ClassDef(PndRegion,1) 
 protected:
 Double_t fZmin;
 Double_t fZmax;
};

#endif



