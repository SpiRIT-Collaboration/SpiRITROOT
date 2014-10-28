
#include "PndFieldCreator.h"

#include "PndFieldMap.h"
#include "PndConstField.h"
#include "PndSolenoidMap.h"
#include "PndDipoleMap.h"
#include "PndTransMap.h"
#include "PndMultiField.h"
#include "PndMultiFieldPar.h"
#include "PndConstPar.h"
#include "PndDipoleMap.h"
#include "PndSolenoidMap.h"
#include "PndDipolePar.h"
#include "PndSolenoidPar.h"
#include "PndTransPar.h"
#include "PndTransMap.h"
#include "PndMapPar.h"
#include "FairBaseParSet.h"
#include "FairRunAna.h"
#include "FairRuntimeDb.h"

#include "TObjArray.h"

using std::cout;
using std::endl;

static PndFieldCreator gPndFieldCreator;


PndFieldCreator::PndFieldCreator()
	:FairFieldFactory(),
     fFieldPar(0),
     fSPar(0),
     fDPar(0),
     fTPar(0),
     fCPar(0),
     fMPar(0)
    
{
	fCreator=this;
 
}
PndFieldCreator::~PndFieldCreator()
{
}
void PndFieldCreator::SetParm()
{
 
//  cout << "PndFieldCreator::SetParm()" <<endl;
  FairRun *Run = FairRun::Instance();
  FairRuntimeDb *RunDB = Run->GetRuntimeDb();
  
  FairBaseParSet* par=(FairBaseParSet*)(RunDB->getContainer("FairBaseParSet"));
 
  TObjArray *contList= par->GetContList();
  TObject *obj=0;
  if (contList) {
	  obj=contList->FindObject("PndMultiFieldPar");
  } 

  cout << "PndFieldCreator::SetParm() " << obj  <<endl;

  	
  if (obj) {
	 fMPar = (PndMultiFieldPar*)RunDB->getContainer("PndMultiFieldPar");
  }else if (contList->FindObject("PndFieldPar")) {
	  fFieldPar = (PndFieldPar*) RunDB->getContainer("PndFieldPar");
  }else if (contList->FindObject("PndSolenoidPar")) {
	  fSPar = (PndSolenoidPar*) RunDB->getContainer("PndSolenoidPar");
  }else if (contList->FindObject("PndDipolePar")) {
	  fDPar = (PndDipolePar*) RunDB->getContainer("PndDipolePar");
  }else if (contList->FindObject("PndTransPar")) {
	  fTPar = (PndTransPar*) RunDB->getContainer("PndTransPar");
  }else if (contList->FindObject("PndConstPar")) {
	  fCPar = (PndConstPar*) RunDB->getContainer("PndConstPar");
  }
	
 
}

FairField* PndFieldCreator::createFairField()
{       
	//cout << "PndFieldCreator::createFairField()" <<endl;
    FairField *fMagneticField=0;
	PndMultiField *MField=0;
	Int_t Type=-1;
    Bool_t multi=kFALSE;
	if (fMPar) {
	   MField = new PndMultiField(fMPar);
	   Type= fMPar->GetType();
	}
	if (Type==-1) {delete MField; MField=0;}
	
	if(MField) {
		multi=kTRUE;
		TObjArray *fParArray=fMPar->GetParArray();
		TIterator *Iter=fParArray->MakeIterator();
   		Iter->Reset();
   		PndMapPar* fPar = NULL;
   		while( (fPar = (PndMapPar*)Iter->Next() ) ) {
			fPar->Print();
			if (fPar->GetType()==0){
				FairField *fField1 = new PndConstField((PndConstPar*)fPar);
				MField->AddField(fField1);
			}
			if (fPar->GetType()==1){
				FairField *fField2 = new PndFieldMap((PndFieldPar*)fPar);
				MField->AddField(fField2);
			}
			if (fPar->GetType()==2){
				FairField *fField3 = new PndSolenoidMap((PndSolenoidPar*)fPar);
				MField->AddField(fField3);
			}
			if (fPar->GetType()==3){
				FairField *fField4 = new PndDipoleMap((PndDipolePar*)fPar);
				MField->AddField(fField4);
			}
			if (fPar->GetType()==4){
				FairField *fField5 = new PndTransMap((PndTransPar*)fPar);
				MField->AddField(fField5);
			}
		}
	}

	if (fCPar) {
	   FairField *fField1 = new PndConstField(fCPar);
	   Type= fCPar->GetType();
	   if (Type==-1) {delete fField1; fField1=0;}
	   if(fField1)fMagneticField=fField1;	
	}
	if(fFieldPar){
	   FairField *fField2 = new PndFieldMap(fFieldPar);
	   Type= fFieldPar->GetType();
	   if (Type==-1){delete fField2; fField2=0;}
	   if(fField2)fMagneticField=fField2;	
    }
	if(fSPar){
	   FairField *fField3 = new PndSolenoidMap(fSPar);
	   Type= fSPar->GetType();
	   if (Type==-1) {delete fField3; fField3=0;}
	   if(fField3)fMagneticField=fField3;
	}
	if(fDPar){
	   FairField *fField4 = new PndDipoleMap(fDPar);
	   Type= fDPar->GetType();
	   if (Type==-1) {delete fField4; fField4=0;}
	   if(fField4)fMagneticField=fField4;	
    }
	if(fTPar){
	   FairField *fField5 = new PndTransMap(fTPar);
	   Type= fTPar->GetType();
	   if (Type==-1){delete fField5; fField5=0;}
	   if(fField5)fMagneticField=fField5;	
    }
        
	if (multi) {
		MField->Init();
		return MField;
	}else{
        if(fMagneticField){
	        fMagneticField->Init();	 
		}
		return fMagneticField;
	}
}


ClassImp(PndFieldCreator)



