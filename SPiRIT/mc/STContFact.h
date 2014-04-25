#ifndef SPIRITCONTFACT_H
#define SPIRITCONTFACT_H

#include "FairContFact.h"

class FairContainer;

class STContFact : public FairContFact
{
  private:
    void setAllContainers();
  public:
    STContFact();
    ~STContFact() {}
    FairParSet* createContainer(FairContainer*);
    ClassDef( STContFact,0) // Factory for all SPiRIT parameter containers
};

#endif
