#include "STContFact.hh"
#include "STGeoPar.hh"
#include "STDigiPar.hh"

#include "FairRuntimeDb.h"

#include <iostream>

ClassImp(STContFact)

static STContFact gSTContFact;

STContFact::STContFact()
:FairContFact()
{
  /** Constructor (called when the library is loaded) */
  fName = "STContFact";
  fTitle = "Factory for parameter containers in libSPiRIT";
  setAllContainers();
  FairRuntimeDb::instance() -> addContFactory(this);
}

STContFact::~STContFact()
{
}

void STContFact::setAllContainers()
{
  /** Creates the Container objects with all accepted
      contexts and adds them to
      the list of containers for the SPiRIT library.
  */

  FairContainer* p = new FairContainer("STGeoPar",
                                       "SPiRIT Geometry Parameters",
                                       "TestDefaultContext");

  containers -> Add(p);

  p = new FairContainer("STDigiPar",
                        "SPiRIT Parameter Container",
                        "");

  containers -> Add(p);
}

FairParSet* STContFact::createContainer(FairContainer* c)
{
  /** Calls the constructor of the corresponding parameter container.
      For an actual context, which is not an empty string and not
      the default context
      of this container, the name is concatinated with the context.
  */
  const char* name = c -> GetName();
  FairParSet* p = NULL;

  if (strcmp(name, "STGeoPar") == 0) {
    p = new STGeoPar(c->getConcatName().Data(),
                     c->GetTitle(),c->getContext());
  }

  if (strcmp(name, "STDigiPar") == 0) {
    p = new STDigiPar(c -> getConcatName().Data(),
                      c -> GetTitle(), c -> getContext());
  }
  return p;
}
