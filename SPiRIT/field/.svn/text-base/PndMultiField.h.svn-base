// -------------------------------------------------------------------------
// -----                      PndMultiField header file                  -----
// -----                 Created 29/01/07  by M. Al/Turany               -----
// -------------------------------------------------------------------------
/** PndMultiField.h
 ** @author M.Al/Turany <m.al-turany@gsi.de>
  ** @since 29.01.2007
 ** @version1.0
 **
 ** Multiple Magnetic field maps 
 **/


#ifndef PNDMULTIMAP_H
#define PNDMULTIMAP_H 1

#include "FairField.h"
#include "TObjArray.h"
#include <map>

class PndRegion;
class PndMultiFieldPar;
class PndMultiField : public FairField {

public:

  /** Default constructor **/
  PndMultiField();

	PndMultiField(TString Map);	
	
	
  /** Constructor from PndFieldPar **/
  PndMultiField(PndMultiFieldPar* fieldPar);

  /** Destructor **/
  virtual ~PndMultiField();

  /** Initialisation (read map from file) **/
  void Init();

  /**Adding a field to the collection*/

  void AddField(FairField *field);
 
  TObjArray *GetFieldList(){return fMaps; }

	
  void FillParContainer();
  /** Screen output **/
  virtual void Print();
  ClassDef(PndMultiField,1) 

 /** Get magnetic field. For use of GEANT3
   ** @param point            Coordinates [cm]
   ** @param bField (return)  Field components [kG] 
   **/
  void GetFieldValue(const Double_t point[3], Double_t* bField);

 protected:
   
  TObjArray *fMaps;
  Int_t     fNoOfMaps;
  std::map <PndRegion*, FairField* > fFieldMaps;//! 
  std::map <PndRegion*, FairField* >::iterator fMapIter; //!

};

#endif



