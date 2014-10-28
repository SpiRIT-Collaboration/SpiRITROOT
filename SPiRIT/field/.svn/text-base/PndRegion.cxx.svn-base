// -------------------------------------------------------------------------
// -----                      PndRegion source file                -----
// -----                 Created 29/01/07  by M. Al/Turany             -----
// -------------------------------------------------------------------------


#include "PndRegion.h"


// -------------   Default constructor  ----------------------------------
PndRegion::PndRegion( Double_t Zmin, Double_t Zmax)
 :TObject()
{
   
  fZmin=Zmin;
  fZmax=Zmax;
}

// ------------   Destructor   --------------------------------------------
PndRegion::~PndRegion() {
}
// ------------   Check if inside this region-------------------------------
 Bool_t PndRegion::IsInside(Double_t Z){
   if( Z>=fZmin && Z<=fZmax ) return kTRUE;
   else return kFALSE;
}

ClassImp(PndRegion)
