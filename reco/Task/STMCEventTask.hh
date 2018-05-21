#ifndef STMCEVENTTASK
#define STMCEVENTTASK

#include "FairTask.h"
#include "FairMCEventHeader.h"

#include "TGraph.h"
#include "TClonesArray.h"

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

   private:
     Bool_t fIsPersistence;

     TClonesArray*        fPrimaryTrackArray;      // from mc.root
     FairMCEventHeader*   fFairMCEventHeader;
     TClonesArray*        fTAMCPointArray;         // from digi.root
     STMCEventHeader*     fMCEventHeader;
     STMCTriggerResponse* fMCTriggerResponse;

     // kyoto cross talk
     Int_t   fNProbBin;
     TGraph* fProbGraph;

   ClassDef(STMCEventTask,1);
};

#endif
