#ifndef STDEDXPOINT_HH
#define STDEDXPOINT_HH

#include "TObject.h"

class STdEdxPoint : public TObject {
  public:
    STdEdxPoint();
    STdEdxPoint(Double_t de, Double_t dx);
    virtual ~STdEdxPoint() {};

    virtual void Print(Option_t *option = "") const;

    Double_t fdE; ///< (ADC)
    Double_t fdx; ///< (mm)

  ClassDef(STdEdxPoint, 1)
};

#endif
