#ifndef STEVENTPREVIEWTASK_hH
#define STEVENTPREVIEWTASK_hH

#include "STRecoTask.hh"
#include "STRawEvent.hh"

#include "STMap.hh"

#include "TH1D.h"
#include "TF1.h"

#define NTB_GGCALIB 100

class STEventPreviewTask : public STRecoTask 
{
  public:
    STEventPreviewTask();
    STEventPreviewTask(Bool_t persistence, Bool_t identifyEvent = false, Bool_t calibrateTb = false);
    ~STEventPreviewTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    virtual void SetParContainers();
    /// Running the task. This will be called when Run() method invoked from FairRun.
    void IdentifyEvent(Bool_t val = true);
    void CalibrateTb(Bool_t val = true);

    /// calculate time offsets for 12 cobos
    void CalculateTbOffsets(STRawEvent *rawEvent, Double_t *tbOffsets);
    void SetAverageBuffer(STRawEvent *rawEvent);
    Double_t FindSecondPeak(Int_t cobo);

    Double_t GetBuffer(Int_t cobo, Int_t tb);

    void SetSkippingEvents(std::vector<Int_t> array);
    void SetSelectingEvents(std::vector<Int_t> array);

  private:
    void LayerTest(STRawEvent *rawEvent);

  private:
    TClonesArray *fRawEventArray = nullptr;

    Bool_t fIdentifyEvent = false;
    Bool_t fCalibrateTb = false;

    STMap *fPadMap = nullptr;

    Double_t fBuffer[12][100];
    Int_t nFilled[12];

    Int_t fNumSkipEvents = 0;
    std::vector<Int_t> fSkipEventArray;

    Int_t fNumSelectedEvents = 0;
    Int_t fSelectedIndex = 0;
    std::vector<Int_t> fSelectedEventArray;

    STDigiPar *fPar = NULL;                    ///< Parameter read-out class pointer
  
  ClassDef(STEventPreviewTask, 2)
};

#endif
