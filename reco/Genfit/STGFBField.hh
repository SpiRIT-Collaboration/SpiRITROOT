#ifndef STGFBFIELD_HH
#define STGFBFIELD_HH

#include "STFieldMap.hh"
#include "AbsBField.h"
#include "TObject.h"
#include "TVector3.h"

class STGFBField : public TObject, public genfit::AbsBField
{
  public:
    static STGFBField *GetInstance(const char* mapName, const char* fileType = "A", Double_t xOffset = -0.1794, Double_t yOffset = -20.5502, Double_t zOffset = 58.0526);

    TVector3 get(const TVector3& pos) const;

    STFieldMap *GetFieldMap() { return fFieldMap; }

  private:
    STGFBField(const char* mapName, const char* fileType = "A", Double_t xOffset = -0.1794, Double_t yOffset = -20.5502, Double_t zOffset = 58.0526);
    virtual ~STGFBField() {};

    static STGFBField *fInstance;
    STFieldMap *fFieldMap;

  ClassDef(STGFBField, 1)
};

#endif
