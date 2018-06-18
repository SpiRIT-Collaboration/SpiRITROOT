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
     void SetCollisionSystem(Int_t ba, Int_t bz, Int_t ta, Int_t tz)
          { fBeamA = ba; fBeamZ = bz; fTargetA = ta; fTargetZ = tz; }
     void SetBeamEnergy(Double_t be) { fBeamE = be; }
     void SetCrosstalkFile(TString file) { fCTFileName = file; }

   private:
     Bool_t fIsPersistence;

     TClonesArray*        fPrimaryTrackArray;      // from mc.root
     FairMCEventHeader*   fFairMCEventHeader;
     TClonesArray*        fTAMCPointArray;         // from digi.root
     STMCEventHeader*     fMCEventHeader;
     STMCTriggerResponse* fMCTriggerResponse;
     TClonesArray*        fScintillatorHitArray;

     Int_t fBeamA;
     Int_t fBeamZ;
     Double_t fBeamE;
     Int_t fTargetA;
     Int_t fTargetZ;

     // kyoto cross talk
     TString fCTFileName;
     Int_t   fNProbBin;
     TGraph* fGraphProb[64][2];
     TH1F* fHistProb[64][2];

     Double_t pbuf[2][50]; // temporary

     Int_t GetChannelFromBar(Int_t);
     void GetPossibleVictim(Int_t, Int_t*);

   ClassDef(STMCEventTask,1);
};

#endif
