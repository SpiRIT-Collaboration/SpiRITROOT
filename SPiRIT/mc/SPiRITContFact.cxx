#include "SPiRITContFact.h"

#include "SPiRITGeoPar.h"

#include "FairRuntimeDb.h"

#include <iostream>

ClassImp(SPiRITContFact)

static SPiRITContFact gSPiRITContFact;

SPiRITContFact::SPiRITContFact()
  : FairContFact()
{
  /** Constructor (called when the library is loaded) */
  fName="SPiRITContFact";
  fTitle="Factory for parameter containers in libSPiRIT";
  setAllContainers();
  FairRuntimeDb::instance()->addContFactory(this);
}

void SPiRITContFact::setAllContainers()
{
  /** Creates the Container objects with all accepted
      contexts and adds them to
      the list of containers for the SPiRIT library.
  */

  FairContainer* p= new FairContainer("SPiRITGeoPar",
                                      "SPiRIT Geometry Parameters",
                                      "TestDefaultContext");
  p->addContext("TestNonDefaultContext");

  containers->Add(p);
}

FairParSet* SPiRITContFact::createContainer(FairContainer* c)
{
  /** Calls the constructor of the corresponding parameter container.
      For an actual context, which is not an empty string and not
      the default context
      of this container, the name is concatinated with the context.
  */
  const char* name=c->GetName();
  FairParSet* p=NULL;
  if (strcmp(name,"SPiRITGeoPar")==0) {
    p=new SPiRITGeoPar(c->getConcatName().Data(),
                            c->GetTitle(),c->getContext());
  }
  return p;
}
