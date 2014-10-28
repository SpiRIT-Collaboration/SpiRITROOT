#include <iostream>
#include "STFieldMap.hh"
#include "STMapPar.hh"
#include "FairParamList.h"


using namespace std;

// ------   Constructor   --------------------------------------------------
STMapPar::STMapPar(const char* name, const char* title,
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

STMapPar::STMapPar() 
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
STMapPar::~STMapPar() { }
// -------------------------------------------------------------------------



// ------   Put parameters   -----------------------------------------------
void STMapPar::putParams(FairParamList* list) {

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
Bool_t STMapPar::getParams(FairParamList* list) {

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
void STMapPar::SetParameters(FairField* field) {

  if ( ! field ) {
    cerr << "-W- STMapPar::SetParameters: Empty field pointer!" << endl;
    return;
  }
   
    STFieldMap* fieldMap = (STFieldMap*) field;
    
    fMapName = field->GetName();
    fPosX   = fieldMap->GetPositionX();
    fPosY   = fieldMap->GetPositionY();
    fPosZ   = fieldMap->GetPositionZ();
    fScale  = fieldMap->GetScale();
    fType   = fieldMap->GetType();
}
// -------------------------------------------------------------------------


ClassImp(STMapPar)

