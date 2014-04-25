#ifndef SPIRITCONTFACT_H
#define SPIRITCONTFACT_H

#include "FairContFact.h"

class FairContainer;

class SPiRITContFact : public FairContFact
{
  private:
    void setAllContainers();
  public:
    SPiRITContFact();
    ~SPiRITContFact() {}
    FairParSet* createContainer(FairContainer*);
    ClassDef( SPiRITContFact,0) // Factory for all SPiRIT parameter containers
};

#endif
