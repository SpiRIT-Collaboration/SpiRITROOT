#include "PndMultiFieldPar.h"
#include "PndMultiFieldPar.h"
#include "PndConstField.h"
#include "PndConstPar.h"
#include "PndDipoleMap.h"
#include "PndSolenoidMap.h"
#include "PndDipolePar.h"
#include "PndSolenoidPar.h"
#include "PndTransPar.h"
#include "PndTransMap.h"
#include "FairParamList.h"
#include "PndMapPar.h"
#include "FairRuntimeDb.h"
#include "FairRun.h"

// ------   Constructor   --------------------------------------------------
PndMultiFieldPar::PndMultiFieldPar(const char* name, const char* title, const char* context) 
  : PndMapPar(name, title, context) 
{

   fParArray=new TObjArray();
}

PndMultiFieldPar::PndMultiFieldPar() 
{
}

PndMultiFieldPar::~PndMultiFieldPar() { }

void PndMultiFieldPar::putParams(FairParamList* list)
{
   if ( ! list ) return;
   list->addObject("List of Field par", fParArray);
   list->add("Field Type", fType);
}



Bool_t PndMultiFieldPar::getParams(FairParamList* l)
{
  if (!l->fillObject("list of fields Par",fParArray))return kFALSE;
  if ( ! l->fill("Field Type", &fType) ) return kFALSE;

  return kTRUE;
} 
void PndMultiFieldPar:: SetParameters(FairField* field)
{	 
   fType=5;
   FairRuntimeDb *rtdb=FairRuntimeDb::instance();
   FairRun *fRun= FairRun::Instance();
   PndMultiField *fmField = (PndMultiField *)field;
   TObjArray *fArray=fmField->GetFieldList();
   TIterator *Iter=fArray->MakeIterator();
   Iter->Reset();
   FairField* fField = NULL;
   Int_t Type=-1;
   while( (fField = (FairField*)Iter->Next() ) ) {
      Type=fField->GetType();
      if(Type==0){
	 PndConstField *fc= (PndConstField *)fField;
	 PndConstPar *cp = (PndConstPar*) rtdb->getContainer("PndConstPar");
	 cp->SetParameters(fc);
         cp->setInputVersion(fRun->GetRunId(),1);
	 fParArray->AddLast(cp);
      }
      if(Type==1){

      } 
      if(Type==2){
         PndSolenoidMap *fs= (PndSolenoidMap *)fField;
	     if (PndSolenoidMap::fNumberOfRegions==1){	 	 	 	 
            PndSolenoidPar* cs = (PndSolenoidPar*) rtdb->getContainer("PndSolenoidPar");
            cs->SetParameters(fs);
	        cs->setInputVersion(fRun->GetRunId(),1);
            fParArray->AddLast(cs);
	     }else{
            TString contN1="PndSolenoid";
	        TString contN3="Par";
            char NO[1];
            sprintf(NO,"%d",fs->GetRegionNo());
            TString contN2=contN1+NO;
            TString contName=contN2+contN3;
	  //    cout << "------"<< "PndMultiFieldPar:: SetParameters(FairField* field) " << contName << endl;
            PndMapPar* cs = (PndMapPar*) rtdb->getContainer(contName.Data());
            cs->SetParameters(fs);
	        cs->setInputVersion(fRun->GetRunId(),1);
            fParArray->AddLast(cs); 
	    }
	 }
	 if(Type==3){
	    PndDipoleMap *fd= (PndDipoleMap *)fField;
        if (PndDipoleMap::fNumberOfRegions==1){
            PndDipolePar* cs = (PndDipolePar*) rtdb->getContainer("PndDipolePar");
            cs->SetParameters(fd);
            cs->setInputVersion(fRun->GetRunId(),1);
            fParArray->AddLast(cs);
         }else{
            TString contN1="PndDipole";
            TString contN3="Par";
            char NO[1];
            sprintf(NO,"%d",fd->GetRegionNo());
            TString contN2=contN1+NO;
            TString contName=contN2+contN3;
            PndMapPar* cs = (PndMapPar*) rtdb->getContainer(contName.Data());
            cs->SetParameters(fd);
            cs->setInputVersion(fRun->GetRunId(),1);
            fParArray->AddLast(cs);
         }
      }
	  if(Type==4){
		 PndTransMap *ft= (PndTransMap *)fField;
         PndTransPar* ct = (PndTransPar*) rtdb->getContainer("PndTransPar");
         ct->SetParameters(ft);
	     ct->setInputVersion(fRun->GetRunId(),1);
	     fParArray->AddLast(ct);
      }
   }
   delete  Iter;
}                      


ClassImp(PndMultiFieldPar)

