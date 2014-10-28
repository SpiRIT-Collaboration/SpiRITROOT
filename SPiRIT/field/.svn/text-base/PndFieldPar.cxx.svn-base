#include <iostream>
#include "PndConstField.h"
#include "PndFieldMap.h"
#include "PndFieldPar.h"
#include "FairParamList.h"
#include "PndMultiField.h"
#include "TObjArray.h"

using namespace std;

// ------   Constructor   --------------------------------------------------
PndFieldPar::PndFieldPar(const char* name, const char* title,
			 const char* context) 
  : FairParGenericSet(name, title, context), 
    fType(-1),
    fXmin(0), 
    fXmax(0),
    fYmin(0), 
    fYmax(0),
    fZmin(0), 
    fZmax(0),
    fBx(0), 
    fBy(0), 
    fBz(0),
    fMapName(TString("")),
    fPosX(0), 
    fPosY(0), 
    fPosZ(0),
    fScale(0)

{
}
// -------------------------------------------------------------------------

PndFieldPar::PndFieldPar() 
  :fType(-1),
  fXmin(0), 
  fXmax(0),
  fYmin(0), 
  fYmax(0),
  fZmin(0), 
  fZmax(0),
  fBx(0), 
  fBy(0), 
  fBz(0),
  fMapName(TString("")),
  fPosX(0), 
  fPosY(0), 
  fPosZ(0),
  fScale(0)
{
}
// -------------------------------------------------------------------------

// ------   Destructor   ---------------------------------------------------
PndFieldPar::~PndFieldPar() { }
// -------------------------------------------------------------------------



// ------   Put parameters   -----------------------------------------------
void PndFieldPar::putParams(FairParamList* list) {

  if ( ! list ) return;

  list->add("Field Type", fType);

  if ( fType == 0 ) {                    // constant field
    list->add("Field min x", fXmin);
    list->add("Field max x", fXmax);
    list->add("Field min y", fYmin);
    list->add("Field max y", fYmax);
    list->add("Field min z", fZmin);
    list->add("Field max z", fZmax);
    list->add("Field Bx", fBx);
    list->add("Field By", fBy);
    list->add("Field Bz", fBz);
  }

  else if (fType >=1 && fType <= 4) {    // field map
    list->add("Field map name", fMapName);
    list->add("Field x position", fPosX);
    list->add("Field y position", fPosY);
    list->add("Field z position", fPosZ);
    list->add("Field scaling factor", fScale);

  }

}
// -------------------------------------------------------------------------



// --------   Get parameters   ---------------------------------------------
Bool_t PndFieldPar::getParams(FairParamList* list) {

  if ( ! list ) return kFALSE;

  if ( ! list->fill("Field Type", &fType) ) return kFALSE;

  if ( fType == 0 ) {                    // constant field
    if ( ! list->fill("Field min x", &fXmin) ) return kFALSE;
    if ( ! list->fill("Field max x", &fXmax) ) return kFALSE;
    if ( ! list->fill("Field min y", &fYmin) ) return kFALSE;
    if ( ! list->fill("Field max y", &fYmax) ) return kFALSE;
    if ( ! list->fill("Field min z", &fZmin) ) return kFALSE;
    if ( ! list->fill("Field max z", &fZmax) ) return kFALSE;
    if ( ! list->fill("Field Bx", &fBx) ) return kFALSE;
    if ( ! list->fill("Field By", &fBy) ) return kFALSE;
    if ( ! list->fill("Field Bz", &fBz) ) return kFALSE;
  }

  else if (fType >=1 && fType <= 4) {    // field map
    Text_t mapName[80];
    if ( ! list->fill("Field map name", mapName, 80) ) return kFALSE;
    fMapName = mapName;
    if ( ! list->fill("Field x position", &fPosX) )  return kFALSE;
    if ( ! list->fill("Field y position", &fPosY) )  return kFALSE;
    if ( ! list->fill("Field z position", &fPosZ) )  return kFALSE;
    if ( ! list->fill("Field scaling factor", &fScale) ) return kFALSE;
  }
  return kTRUE;

}
// -------------------------------------------------------------------------



// ---------   Set parameters from FairField   ------------------------------
void PndFieldPar::SetParameters(FairField* field) {

  if ( ! field ) {
    cerr << "-W- PndFieldPar::SetParameters: Empty field pointer!" << endl;
    return;
  }

  fType = field->GetType();
 
  if ( fType == 0 ) {                                 // constant field
    PndConstField* fieldConst = (PndConstField*) field;
    fBx = fieldConst->GetBx();
    fBy = fieldConst->GetBy();
    fBz = fieldConst->GetBz();
    fXmin = fieldConst->GetXmin();
    fXmax = fieldConst->GetXmax();
    fYmin = fieldConst->GetYmin();
    fYmax = fieldConst->GetYmax();
    fZmin = fieldConst->GetZmin();
    fZmax = fieldConst->GetZmax();
    fMapName = "";
    fPosX = fPosY = fPosZ = fScale = 0.;
  }

  else if ( fType >=1 && fType <= 4 ) {              // field map
    PndFieldMap* fieldMap = (PndFieldMap*) field;
    fBx = fBy = fBz = 0.;
    fXmin = fXmax = fYmin = fYmax = fZmin = fZmax = 0.;
    
    fMapName = field->GetName();
    fPosX   = fieldMap->GetPositionX();
    fPosY   = fieldMap->GetPositionY();
    fPosZ   = fieldMap->GetPositionZ();
    fScale  = fieldMap->GetScale();
  } else if (fType ==5) {
    
    PndMultiField *fMulti=(PndMultiField *)field;
    TObjArray *fieldlist = fMulti->GetFieldList();
    TIterator* FieldIter = fieldlist->MakeIterator();
    FieldIter->Reset();
    FairField *fi=0;
    while( (fi = (FairField*)FieldIter->Next() ) ) {
         SetParameters(fi);
    }
   
   delete  FieldIter;
  }else {
    cerr << "-W- PndFieldPar::SetParameters: Unknown field type "
	 << fType << "!" << endl;
    fBx = fBy = fBz = 0.;
    fXmin = fXmax = fYmin = fYmax = fZmin = fZmax = 0.;
    fMapName = "";
    fPosX = fPosY = fPosZ = fScale = 0.;
  }
  
  return;

}
// -------------------------------------------------------------------------


    
    
    

ClassImp(PndFieldPar)

