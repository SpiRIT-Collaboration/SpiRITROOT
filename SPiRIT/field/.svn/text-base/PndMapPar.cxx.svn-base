#include <iostream>
#include "PndFieldMap.h"
#include "PndMapPar.h"
#include "FairParamList.h"


using namespace std;

// ------   Constructor   --------------------------------------------------
PndMapPar::PndMapPar(const char* name, const char* title,
			 const char* context) 
  : FairParGenericSet(name, title, context),
    fType(-1),
    fXmin(0),
    fXmax(0),
    fYmin(0),
    fYmax(0),
    fZmin(0),
    fZmax(0),
    fMapName(TString("")),
    fPosX(0),
    fPosY(0),
    fPosZ(0),
    fScale(0)
{
}
// -------------------------------------------------------------------------

PndMapPar::PndMapPar() 
   :fType(-1),
   fXmin(0),
   fXmax(0),
   fYmin(0),
   fYmax(0),
   fZmin(0),
   fZmax(0),
   fMapName(TString("")),
   fPosX(0),
   fPosY(0),
   fPosZ(0),
   fScale(0)
{
  
}
// -------------------------------------------------------------------------

// ------   Destructor   ---------------------------------------------------
PndMapPar::~PndMapPar() { }
// -------------------------------------------------------------------------



// ------   Put parameters   -----------------------------------------------
void PndMapPar::putParams(FairParamList* list) {

  if ( ! list ) return;

  list->add("Field Type", fType);
  list->add("Field map name", fMapName);
  list->add("Field x position", fPosX);
  list->add("Field y position", fPosY);
  list->add("Field z position", fPosZ);
  list->add("Field scaling factor", fScale);
}
// -------------------------------------------------------------------------



// --------   Get parameters   ---------------------------------------------
Bool_t PndMapPar::getParams(FairParamList* list) {

  if ( ! list ) return kFALSE;

  if ( ! list->fill("Field Type", &fType) ) return kFALSE;

  Text_t mapName[80];
  if ( ! list->fill("Field map name", mapName, 80) ) return kFALSE;
  fMapName = mapName;
  if ( ! list->fill("Field x position", &fPosX) )  return kFALSE;
  if ( ! list->fill("Field y position", &fPosY) )  return kFALSE;
  if ( ! list->fill("Field z position", &fPosZ) )  return kFALSE;
  if ( ! list->fill("Field scaling factor", &fScale) ) return kFALSE;
  return kTRUE;

}
// -------------------------------------------------------------------------



// ---------   Set parameters from FairField   ------------------------------
void PndMapPar::SetParameters(FairField* field) {

  if ( ! field ) {
    cerr << "-W- PndMapPar::SetParameters: Empty field pointer!" << endl;
    return;
  }
   
    PndFieldMap* fieldMap = (PndFieldMap*) field;
    
    fMapName = field->GetName();
    fPosX   = fieldMap->GetPositionX();
    fPosY   = fieldMap->GetPositionY();
    fPosZ   = fieldMap->GetPositionZ();
    fScale  = fieldMap->GetScale();
    fType   = fieldMap->GetType();
}
// -------------------------------------------------------------------------


ClassImp(PndMapPar)

