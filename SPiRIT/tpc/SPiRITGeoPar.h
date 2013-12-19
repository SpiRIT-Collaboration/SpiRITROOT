#ifndef SPIRITGEOPAR_H
#define SPIRITGEOPAR_H

#include "FairParGenericSet.h"

class TObjArray;
class FairParamList;

class SPiRITGeoPar       : public FairParGenericSet
{
  public:

    /** List of FairGeoNodes for sensitive  volumes */
    TObjArray*      fGeoSensNodes;

    /** List of FairGeoNodes for sensitive  volumes */
    TObjArray*      fGeoPassNodes;

    SPiRITGeoPar(const char* name="SPiRITGeoPar",
                      const char* title="SPiRIT Geometry Parameters",
                      const char* context="TestDefaultContext");
    ~SPiRITGeoPar(void);
    void clear(void);
    void putParams(FairParamList*);
    Bool_t getParams(FairParamList*);
    TObjArray* GetGeoSensitiveNodes() {return fGeoSensNodes;}
    TObjArray* GetGeoPassiveNodes()   {return fGeoPassNodes;}

  private:
    SPiRITGeoPar(const SPiRITGeoPar&);
    SPiRITGeoPar& operator=(const SPiRITGeoPar&);

    ClassDef(SPiRITGeoPar,1)
};

#endif
