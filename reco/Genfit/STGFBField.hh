#ifndef STGFBFIELD_HH
#define STGFBFIELD_HH

#include "STFieldMap.hh"
#include "AbsBField.h"
#include "TObject.h"
#include "TVector3.h"

class STGFBField : public TObject, public genfit::AbsBField
{
  public:
    static STGFBField *GetInstance(const char* mapName, const char* fileType = "R", Double_t xOffset = 0, Double_t yOffset = -20.43, Double_t zOffset = 58);

    TVector3 get(const TVector3& pos) const;

    STFieldMap *GetFieldMap() { return fFieldMap; }

  private:
    STGFBField(const char* mapName, const char* fileType = "R", Double_t xOffset = 0, Double_t yOffset = -20.43, Double_t zOffset = 58);
    virtual ~STGFBField() {};

    static STGFBField *fInstance;
    STFieldMap *fFieldMap;

  ClassDef(STGFBField, 1)
};

#endif
