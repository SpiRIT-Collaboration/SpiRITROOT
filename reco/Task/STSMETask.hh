#ifndef STSMETASK
#define STSMETASK

#include "STRecoTask.hh"
#include "STSystemManipulator.hh"

class STSMETask : public STRecoTask
{
  public:
    STSMETask();
    STSMETask(Bool_t persistence);
    ~STSMETask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

  private:
    TClonesArray *fClusterArray = nullptr;
    TClonesArray *fSMClusterArray = nullptr;

    STSystemManipulator *fManipulator;

  ClassDef(STSMETask, 1)
};

#endif
