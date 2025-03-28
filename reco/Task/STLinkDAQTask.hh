#ifndef STLINKDAQTASK_H
#define STLINKDAQTASK_H
/*
 * Task to link events from the FRIBDAQ and the processed BDC files
 * 
 * Based on AtLinkDAQTask by Adam Anthony from ATTPCROOT
 *
 */

#include "STAuxHeader.hh"

#include <TChain.h>
#include <TString.h>
#include <TTree.h>
// FairRoot classes
#include <FairTask.h>

#include <Rtypes.h>

#include <memory>
#include <vector>

class STLinkDAQTask : public FairTask {

private:
   // Info for SpiRITROOT Tree
   STAuxHeader *fInputAuxHeader;       // AtRawEvent
   TString fInputBranchName{"STAuxHeader"}; // Name if AtRawEvent branch
   STAuxHeader *fOutputAuxHeader;       // AtRawEvent
   TString fOutputBranchName{"STAuxHeaderLinked"}; // Name if AtRawEvent branch

   // Info for BDC Tree input
   TChain *bdcTree{nullptr};
   ULong64_t fBdcTS{0};
   TString fBdcTimeName{"TimeStamp"};

   // Info for output Beam Info tree

   // Variables used during merging
   ULong64_t fBdcTreeIndex{0};
   ULong64_t fOldBdcTimestamp{0};
   ULong64_t fBdcTimestamp{0};
   Double_t fIntervalBdc{0};

   ULong64_t fTpcTreeIndex{0};
   ULong_t fOldTpcTimestamp{0};
   ULong_t fTpcTimestamp{0};
   Double_t fIntervalTpc{0};

   std::vector<ULong64_t> fBdcIntervals;
   std::vector<ULong64_t> fTpcIntervals;

   // Input parameters
   Double_t fSearchMean{0}; // This is the ratio of clock frequencies
   Double_t fSearchRadius{0};

   Bool_t kPersistent{false};
   Bool_t kBeamInfoPersistent{false};
   Bool_t kFirstEvent{true};

   Double_t GetScaledInterval(ULong64_t intervalBDC, ULong64_t intervalTPC);

   Int_t fFirstEvent{0};

   void DoFirstEvent();
   Int_t SyncStart();
   bool UpdateTimestamps();
   void ResetFlags();
   void Fill();
   Int_t CheckMatch();

public:
   STLinkDAQTask() = default;
   ~STLinkDAQTask() = default;

   bool SetInputTree(TString fileName, TString treeName);
   bool AddInputTree(TString fileName);
   void SetPersistence(Bool_t val) { kPersistent = val; }
   void SetInputBranch(TString name) { fInputBranchName = name; }
   void SetOutputBranch(TString name) { fOutputBranchName = name; }
   void SetBdcTimestamp(TString name) { fBdcTimeName = name; }

   void SetSearchRadius(Double_t radius) { fSearchRadius = radius; }

   void SetFirstEvent(Int_t val) { fFirstEvent = val; }

   virtual InitStatus Init() override;
   virtual void Exec(Option_t *opt) override;
};
#endif //#define STLINKDAQTASK_H
