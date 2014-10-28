// -------------------------------------------------------------------------
// -----                      PndMultiField source file                -----
// -----                 Created 29/01/07  by M. Al/Turany             -----
// -------------------------------------------------------------------------



#include "PndMultiField.h"
#include "PndRegion.h"
#include "PndConstField.h"
#include "PndFieldMap.h"
#include "PndMapPar.h"
#include "PndMultiFieldPar.h"
#include "PndSolenoidMap.h"
#include "PndTransMap.h"
#include "PndDipoleMap.h"


#include "FairRun.h"
#include "FairRuntimeDb.h"


#include "TObjArray.h"



#include <iomanip>
#include <iostream>
#include <fstream>



using namespace std;

// -------------   Default constructor  ----------------------------------
PndMultiField::PndMultiField() {

    fMaps= new TObjArray(10);
    fNoOfMaps=0;
    fType = 5;
}


// -------------   Default constructor  ----------------------------------
PndMultiField::PndMultiField(TString Map) {
	
    fMaps= new TObjArray(10);
    fNoOfMaps=0;
    fType = 5;
	
	Map.ToUpper();
	
	if (Map=="FULL") {
		
	    PndTransMap *map_t= new PndTransMap("TransMap", "R");
		PndDipoleMap *map_d1= new PndDipoleMap("DipoleMap1", "R");
		PndDipoleMap *map_d2= new PndDipoleMap("DipoleMap2", "R");
		PndSolenoidMap *map_s1= new PndSolenoidMap("SolenoidMap1", "R");
		PndSolenoidMap *map_s2= new PndSolenoidMap("SolenoidMap2", "R");
		PndSolenoidMap *map_s3= new PndSolenoidMap("SolenoidMap3", "R");
		PndSolenoidMap *map_s4= new PndSolenoidMap("SolenoidMap4", "R");
		
		AddField(map_t);
		AddField(map_d1);
		AddField(map_d2);
		AddField(map_s1);
		AddField(map_s2);
		AddField(map_s3);
		AddField(map_s4);
	
	}else if (Map="DIPOLE") {
		PndDipoleMap *map_d1= new PndDipoleMap("DipoleMap1", "R");
		PndDipoleMap *map_d2= new PndDipoleMap("DipoleMap2", "R");
		
		AddField(map_d1);
		AddField(map_d2);
		
	}else if (Map=="SOLENOID") {
		
		PndSolenoidMap *map_s1= new PndSolenoidMap("SolenoidMap1", "R");
		PndSolenoidMap *map_s2= new PndSolenoidMap("SolenoidMap2", "R");
		PndSolenoidMap *map_s3= new PndSolenoidMap("SolenoidMap3", "R");
		PndSolenoidMap *map_s4= new PndSolenoidMap("SolenoidMap4", "R");
		
		AddField(map_s1);
		AddField(map_s2);
		AddField(map_s3);
		AddField(map_s4);
	}

	

}




// ------------------------------------------------------------------------

// ------------   Constructor from PndFieldPar   --------------------------
PndMultiField::PndMultiField(PndMultiFieldPar* fieldPar) {
   fType = 5;
   fMaps= new TObjArray(10);
   fNoOfMaps=0;
   TObjArray *fArray= fieldPar->GetParArray();
   if(fArray->IsEmpty()) fType=-1;

}

// ------------   Destructor   --------------------------------------------
PndMultiField::~PndMultiField() {
	
	//printf("PndMultiField::~PndMultiField() \n");
	fMaps->Delete();
	delete fMaps;
	
}

// -----------   Adding fields   ------------------------------------------
void PndMultiField::AddField(FairField *field){
	
   if(field){
      fMaps->AddLast(field);	
      fNoOfMaps++;
   }

}

// -----------   Intialisation   ------------------------------------------
void PndMultiField::Init() {
   PndConstField *field=0;
   PndFieldMap *fieldMap=0;
   for (Int_t n=0; n<=fNoOfMaps; n++){
      fieldMap = dynamic_cast<PndFieldMap *>(fMaps->At(n));
      field = dynamic_cast<PndConstField *>(fMaps->At(n));
      if(fieldMap){
         fieldMap->Init();
	 fFieldMaps.insert( pair<PndRegion*, FairField*>(new PndRegion(fieldMap->GetZmin(),fieldMap->GetZmax()) , fieldMap ));
      }else if(field){
         field->Init();
	 fFieldMaps.insert( pair<PndRegion*, FairField*>(new PndRegion(field->GetZmin(),field->GetZmax() ), field ));
      }
   }

}

// ---------   Screen output   --------------------------------------------
void PndMultiField::Print() {  
	for (Int_t n=0; n<=fNoOfMaps; n++){
		FairField *fieldMap = dynamic_cast<FairField *>(fMaps->At(n));
		if(fieldMap) fieldMap->Print();
	}
}





// ---------   Fill the parameters   --------------------------------------------
void PndMultiField::FillParContainer() {  
// for (Int_t n=0; n<=fNoOfMaps; n++){
//      FairField *fieldMap = dynamic_cast<FairField *>(fMaps->At(n));
//      if(fieldMap) fieldMap->FillParContainer();
//  }
  FairRun *fRun=FairRun::Instance();	
  FairRuntimeDb *rtdb=fRun->GetRuntimeDb();
  Bool_t kParameterMerged=kTRUE;
  PndMultiFieldPar* Par = (PndMultiFieldPar*) rtdb->getContainer("PndMultiFieldPar");
  Par->SetParameters(this);
  Par->setInputVersion(fRun->GetRunId(),1);
  Par->setChanged();
	
	
	
}

// -------------------------------------------------------------------------
void PndMultiField::GetFieldValue(const Double_t point[3], Double_t* bField) 
{
  
   PndRegion *fReg=0;
   FairField *fField=0;
   for (fMapIter=fFieldMaps.begin(); fMapIter!= fFieldMaps.end();fMapIter++ ){
      fReg=fMapIter->first;
      if(fReg->IsInside(point[2])){
         fField=fMapIter->second;
	 break;
      }
   }
  if(fField){
    fField->GetBxyz(point, bField);
  }else{
    bField[0] = 0;
    bField[1] = 0;
    bField[2] = 0;

  }
}


ClassImp(PndMultiField)
