#ifndef STCONTFACT_H
#define STCONTFACT_H

#include "FairContFact.h"

class FairContainer;

class STContFact : public FairContFact
{
  public:
    STContFact();
    ~STContFact();

    FairParSet* createContainer(FairContainer*);

  private:
    void setAllContainers();

  ClassDef(STContFact, 1) // Factory for all SPiRIT parameter containers
};

#endif
