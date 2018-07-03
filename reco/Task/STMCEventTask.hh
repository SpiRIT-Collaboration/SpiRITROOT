#ifndef STMCEVENTTASK
#define STMCEVENTTASK

#include "FairTask.h"
#include "FairMCEventHeader.h"

#include "TGraph.h"
#include "TH1F.h"
#include "TClonesArray.h"
#include "TString.h"

#include "STMCEventHeader.hh"
#include "STMCTriggerResponse.hh"

class STMCEventTask : public FairTask
{
  public:
    STMCEventTask();
    virtual ~STMCEventTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t* opt);
    virtual void SetParContainers();
    void SetPersistence(Bool_t value = kTRUE);

    void SetCrosstalkFile(TString file) { fCTFileName = file; }

  private:
    Bool_t fIsPersistence;

    TClonesArray*        fPrimaryTrackArray;      // from mc.root
    FairMCEventHeader*   fFairMCEventHeader;
    TClonesArray*        fTAMCPointArray;         // from digi.root
    STMCEventHeader*     fMCEventHeader;
    STMCTriggerResponse* fMCTriggerResponse;
    TClonesArray*        fScintillatorHitArray;

    Bool_t               fIsSetSystem;
    TClonesArray*        fBeamVectorArray;
    TClonesArray*        fTargetVectorArray;

    // kyoto array 
    Double_t fkB;   // birk's coefficient
    Double_t fC;    // second order coeff.
    Double_t fTopOfBar;   // Y position of bar
    Double_t fAttLength;  // attenuation length
    TString fCTFileName;
    TGraph* fGraphProb[64][2];
    TH1F* fHistProb[64][2];
    void ReadCrosstalkFile();


    Int_t GetChannelFromBar(Int_t);
    void GetPossibleVictim(Int_t, Int_t*);

    ClassDef(STMCEventTask,1);
};

#endif
