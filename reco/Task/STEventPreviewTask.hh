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

    void IdentifyEvent(Bool_t val = true);
    void CalibrateTb(Bool_t val = true);

    /// Find reference timebucket from section0(tbRef0),
    /// calculate offsets for other 3 sections(tbOffsets[3])
    void CalculateTbOffsets(STRawEvent *rawEvent, Double_t *tbOffsets);
    void SetAverageHistograms(STRawEvent *rawEvent);
    TH1D *GetAverageHist(Int_t coboIdx); ///< draw average histogram for selected coboIdx
    TF1 *FitGG(TH1D *hist); ///< landau fit
    Double_t FindSecondPeak(TH1D *hist);

  private:
    void LayerTest(STRawEvent *rawEvent);

  private:
    TClonesArray *fRawEventArray = nullptr;

    Bool_t fIdentifyEvent = false;
    Bool_t fCalibrateTb = false;

    STMap *fPadMap = nullptr;

    TH1D *fHistGG[12];
    TF1 *fF1GG = nullptr;

  ClassDef(STEventPreviewTask, 2)
};

#endif
