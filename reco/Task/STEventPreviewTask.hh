#ifndef STEVENTPREVIEWTASK_hH
#define STEVENTPREVIEWTASK_hH

#include "STRecoTask.hh"
#include "STRawEvent.hh"

class STEventPreviewTask : public STRecoTask 
{
  public:
    STEventPreviewTask();
    STEventPreviewTask(Bool_t persistence);

    ~STEventPreviewTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

  private:
    void LayerTest(STRawEvent *rawEvent);

  private:
    TClonesArray *fRawEventArray = nullptr;

  ClassDef(STEventPreviewTask, 1)
};

#endif
