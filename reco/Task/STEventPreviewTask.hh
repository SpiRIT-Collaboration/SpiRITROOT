#ifndef STEVENTPREVIEWTASK_hH
#define STEVENTPREVIEWTASK_hH

#include "STRecoTask.hh"
#include "STRawEvent.hh"

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

    void IdentifyEvent(Bool_t val = true);
    void CalibrateTb(Bool_t val = true);

    /// Find reference timebucket from section0(tbRef0),
    /// calculate offsets for other 3 sections(tbOffsets[3])
    void CalculateTbOffsets(STRawEvent *rawEvent, Double_t &tbRef0, Double_t *tbOffsets);
    TH1D *GetAverageHist(STRawEvent *rawEvent, Int_t section); ///< draw average histogram for selected section
    TF1 *FitGG(TH1D *hist); ///< landau fit

  private:
    void LayerTest(STRawEvent *rawEvent);

  private:
    TClonesArray *fRawEventArray = nullptr;

    Bool_t fIdentifyEvent = false;
    Bool_t fCalibrateTb = false;

    TH1D *fHistGG[4];
    TF1 *fF1GG = nullptr;

  ClassDef(STEventPreviewTask, 2)
};

#endif
