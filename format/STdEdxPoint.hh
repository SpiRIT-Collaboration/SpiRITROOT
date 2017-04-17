#ifndef STDEDXPOINT_HH
#define STDEDXPOINT_HH

#include "TObject.h"

class STdEdxPoint : public TObject {
  public:
    STdEdxPoint();
    STdEdxPoint(Int_t groupID, Double_t de, Double_t dx);
    virtual ~STdEdxPoint() {};

    Int_t fGroupID; ///< layerID for (+), rowID for (-), non for >900
    Double_t fdE; ///< (ADC)
    Double_t fdx; ///< (mm)

  ClassDef(STdEdxPoint, 1)
};

#endif
