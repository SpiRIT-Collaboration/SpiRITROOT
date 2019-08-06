#ifndef STNLDIGITASK
#define STNLDIGITASK

#include "FairTask.h"
#include "STMCPoint.hh"
#include "TClonesArray.h"
#include "TGeoTrack.h"

#include "STNeuLAND.hh"

#include <map>
using namespace std;

class STNLDigiTask : public FairTask
{
  public:
    STNLDigiTask(TString name = "");
    ~STNLDigiTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t* opt);
    virtual void SetParContainers();

    virtual void FinishEvent();
    virtual void FinishTask();

    void SetBarPersistence(Bool_t value);
    void SetHitPersistence(Bool_t value);
    void SetHitClusterPersistence(Bool_t value);

    void CopyGeoTrackFromTo(TGeoTrack *from, TGeoTrack *to);

    void SetThreshold(Double_t threshold);

    void CreateSummary(Bool_t make) { fCreateSummary = make; }

  private:
    TString fName;
    Int_t fEventID;

    TClonesArray* fMCPointArray;
    TClonesArray* fMCPointArrayOut;

    TClonesArray* fGeoTrack;
    TClonesArray* fGeoTrackOut;

    TClonesArray* fBarArray;
    TClonesArray* fNLHitArray;
    TClonesArray* fNLHitClusterArray;

    STNeuLAND *fNL;

    Bool_t fIsBarPersistence = true;
    Bool_t fIsHitPersistence = true;
    Bool_t fIsHitClusterPersistence = true;

    Double_t fThreshold = 0.1;

    //

    Bool_t fCreateSummary = true;
    Bool_t fSummaryIsGoodToFill = true;

    TFile *fSummaryFile = nullptr;

    TTree *fSummaryTreeEvent = nullptr;
    Double_t fSummaryQReco = 0.;
    Double_t fSummaryQMC = 0.;
     Short_t fSummaryNumBars = 0;
     Short_t fSummaryNumHits = 0;
     Short_t fSummaryNumClusters = 0;
     Short_t fSummaryNumMCPoints = 0;

    TTree *fSummaryTreeHit = nullptr;
    TVector3 fSummaryPosHit;

    TTree *fSummaryTreeMC = nullptr;
    TVector3 fSummaryPosMC;

    TTree *fSummaryTreeDist = nullptr;
    Double_t fSummaryTwoHitDist = -1;

    //

  ClassDef(STNLDigiTask,1);
};

#endif
