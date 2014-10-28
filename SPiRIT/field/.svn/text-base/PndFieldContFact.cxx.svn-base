#include "PndFieldContFact.h"
#include "PndFieldPar.h"
#include "PndSolenoidPar.h"
#include "PndSolenoid1Par.h"
#include "PndSolenoid2Par.h"
#include "PndSolenoid3Par.h"
#include "PndSolenoid4Par.h"
#include "PndDipolePar.h"
#include "PndDipole1Par.h"
#include "PndDipole2Par.h"
#include "PndDipole3Par.h"
#include "PndTransPar.h"
#include "PndConstPar.h"
#include "FairParSet.h"
#include "FairRuntimeDb.h"
#include "PndMultiFieldPar.h"
#include <iostream>

using namespace std;
static PndFieldContFact gPndFieldContFact;


// -----   Constructor   ---------------------------------------------------
PndFieldContFact::PndFieldContFact() {
  fName = "PndFieldContFact";
  fTitle = "Factory for field parameter containers";
  SetAllContainers();
  FairRuntimeDb::instance()->addContFactory(this);
}
// -------------------------------------------------------------------------


// -----   Destructor   ----------------------------------------------------
PndFieldContFact::~PndFieldContFact() { }
// -------------------------------------------------------------------------



// -----   Create containers   ---------------------------------------------
FairParSet* PndFieldContFact::createContainer(FairContainer* container) {


  const char* name = container->GetName();
 // cout << "create PndFieldPar container " << name << endl;
  FairParSet* set = NULL;
  if ( strcmp(name, "PndFieldPar") == 0 ) {
    set = new PndFieldPar( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  } else if ( strcmp(name, "PndSolenoidPar") == 0 ) {
    set = new PndSolenoidPar( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  } else if ( strcmp(name, "PndSolenoid1Par") == 0 ) {
    set = new PndSolenoid1Par( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  } else if ( strcmp(name, "PndSolenoid2Par") == 0 ) {
    set = new PndSolenoid2Par( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  } else if ( strcmp(name, "PndSolenoid3Par") == 0 ) {
    set = new PndSolenoid3Par( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  } else if ( strcmp(name, "PndSolenoid4Par") == 0 ) {
    set = new PndSolenoid4Par( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );			   			   			   			   
  } else if ( strcmp(name, "PndDipolePar") == 0 ) {
    set = new PndDipolePar( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  } else if ( strcmp(name, "PndDipole1Par") == 0 ) {
    set = new PndDipole1Par( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  } else if ( strcmp(name, "PndDipole2Par") == 0 ) {
    set = new PndDipole2Par( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  } else if ( strcmp(name, "PndDipoleP3ar") == 0 ) {
    set = new PndDipole3Par( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  } else if ( strcmp(name, "PndTransPar") == 0 ) {
    set = new PndTransPar( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  } else if ( strcmp(name, "PndConstPar") == 0 ) {
    set = new PndConstPar( container->getConcatName().Data(),
			   container->GetTitle(),
			   container->getContext() );
  }else if ( strcmp(name, "PndMultiFieldPar") == 0 ) {
    set = new PndMultiFieldPar( container->getConcatName().Data(),
 			   container->GetTitle(),	
			   container->getContext() );
  }  
return set;
}
// -------------------------------------------------------------------------



// -----   Set all containers (private)   ----------------------------------
void PndFieldContFact::SetAllContainers() {
  FairContainer* con1 = new FairContainer("PndFieldPar",
					     "Field parameter container",
					     "Default field");
  containers->Add(con1);
  FairContainer* con2 = new FairContainer("PndSolenoidPar",
					     "Solenoid Field parameter container",
					     "Default field");
  containers->Add(con2);
 
  FairContainer* con21 = new FairContainer("PndSolenoid1Par",
					     "Solenoid 1st region parameter container",
					     "Default field");
  containers->Add(con21);
  FairContainer* con22 = new FairContainer("PndSolenoid2Par",
					     "Solenoid 2nd region parameter container",
					     "Default field");
  containers->Add(con22);					  
  
  FairContainer* con23 = new FairContainer("PndSolenoid3Par",
					     "Solenoid 3rd region parameter container",
					     "Default field");
  containers->Add(con23);
  
  FairContainer* con24 = new FairContainer("PndSolenoid4Par",
					     "Solenoid 4th region parameter container",
					     "Default field");					     					     					     					     
  containers->Add(con24);
  
  FairContainer* con3 = new FairContainer("PndDipolePar",
					     "Dipole Field parameter container",
					     "Default field");
  containers->Add(con3);
  
  
  FairContainer* con31 = new FairContainer("PndDipole1Par",
					     "Dipole Field parameter container",
					     "Default field");
  containers->Add(con31);
  
  FairContainer* con32 = new FairContainer("PndDipole2Par",
					     "Dipole Field parameter container",
					     "Default field");
  containers->Add(con32);
  
 
  FairContainer* con33 = new FairContainer("PndDipole3Par",
					     "Dipole Field parameter container",
					     "Default field");
  containers->Add(con33);
  
    
  FairContainer* con4 = new FairContainer("PndTransPar",
					     "Trans. Field parameter container",
					     "Default field");
  containers->Add(con4);

  FairContainer* con5 = new FairContainer("PndConstPar",
					     "Const Field parameter container",
					     "Default field");
  containers->Add(con5);

  FairContainer* con6 = new FairContainer("PndMultiFieldPar",
					     "Multiple Field parameter container",
					     "Default field");
  containers->Add(con6);


}
// -------------------------------------------------------------------------


ClassImp(PndFieldContFact)
