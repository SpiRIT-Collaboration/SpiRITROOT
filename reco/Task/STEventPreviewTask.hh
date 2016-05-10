#ifndef STEVENTPREVIEWTASK_hH
#define STEVENTPREVIEWTASK_hH

#include "STRecoTask.hh"
#include "STRawEvent.hh"

class STEventPreviewTask : public STRecoTask 
{
  public:
    STEventPreviewTask();
    STEventPreviewTask(Bool_t persistence, Bool_t identifyEvent = false);

    ~STEventPreviewTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

  private:
    void LayerTest(STRawEvent *rawEvent);

  private:
    TClonesArray *fRawEventArray = nullptr;

    Bool_t fIdentifyEvent = false;

  ClassDef(STEventPreviewTask, 1)
};

#endif
