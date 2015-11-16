// =================================================
//  STPedestal Class
// 
//  Description:
//    This class is used for calculating pedestal
//    values of each channel using FPN channels.
// 
//  Genie Jhang ( geniejhang@majimak.com )
//  2015. 11. 14
// =================================================

#ifndef STPEDESTAL
#define STPEDESTAL

#include "TObject.h"

#include "GETMath.hh"

#include <fstream>

class STPedestal : public TObject {
  public:
    STPedestal();

    Bool_t SubtractPedestal(Int_t numTbs, Int_t *fpn, Int_t *rawADC, Double_t *dest, Double_t rmsCut = 5, Bool_t signalNegativePolarity = kTRUE, Int_t startTb = 3, Int_t averageTbs = 10);

  private:
    GETMath *fMath;

  ClassDef(STPedestal, 1);
};

#endif
