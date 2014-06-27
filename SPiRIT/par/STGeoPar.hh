#ifndef STGEOPAR_H
#define STGEOPAR_H

#include "FairParGenericSet.h"

class TObjArray;
class FairParamList;

class STGeoPar       : public FairParGenericSet
{
  public:

    /** List of FairGeoNodes for sensitive  volumes */
    TObjArray*      fGeoSensNodes;

    /** List of FairGeoNodes for sensitive  volumes */
    TObjArray*      fGeoPassNodes;

    STGeoPar(const char* name="STGeoPar",
             const char* title="SPiRIT Geometry Parameters",
             const char* context="TestDefaultContext");
    ~STGeoPar(void);
    void clear(void);
    void putParams(FairParamList*);
    Bool_t getParams(FairParamList*);
    TObjArray* GetGeoSensitiveNodes() {return fGeoSensNodes;}
    TObjArray* GetGeoPassiveNodes()   {return fGeoPassNodes;}

  private:
    STGeoPar(const STGeoPar&);
    STGeoPar& operator=(const STGeoPar&);

    ClassDef(STGeoPar,1)
};

#endif
