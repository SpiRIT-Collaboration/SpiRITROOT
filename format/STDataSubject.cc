#include "STDataSubject.hh"

#include "STDataObserver.hh"

using namespace DataHandling;

void STSubject::Notify()
{
   for (auto obs : fObservers)
      obs->Update(this);
}
void STSubject::Attach(STObserver *observer)
{
   fObservers.insert(observer);
}
