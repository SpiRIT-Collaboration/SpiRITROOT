#ifndef STGFBFIELD_HH
#define STGFBFIELD_HH

#include "STFieldMap.hh"
#include "AbsBField.h"
#include "TObject.h"
#include "TVector3.h"

class STGFBField : public TObject, public genfit::AbsBField
{
  public:
    STGFBField(const char* mapName, const char* fileType = "R");
    virtual ~STGFBField() {};

    static STGFBField *GetInstance(const char* mapName, const char* fileType = "R");

    TVector3 get(const TVector3& pos) const;

    STFieldMap *GetFieldMap() { return fFieldMap; }

  private:
    static STGFBField *fInstance;
    STFieldMap *fFieldMap;

  ClassDef(STGFBField, 1)
};

#endif
