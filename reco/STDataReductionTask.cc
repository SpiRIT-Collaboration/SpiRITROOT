#include "STDataReductionTask.hh"

#include "STRawEvent.hh"

#include <FairLogger.h>
#include <FairRootManager.h>
#include <FairRunAna.h>
#include <FairTask.h>

#include <TClonesArray.h>
#include <TObject.h>

ClassImp(STDataReductionTask);

void STDataReductionTask::SetReductionFunction(std::function<bool(STRawEvent *)> func)
{
   // Bind the reduction function to the passed function, having it call that function with the current
   // AtRawEvent in the pointer this->fRawEvent.
   fReductionFunc = [this, func]() { return func(fEvent); };
}

InitStatus STDataReductionTask::Init()
{
   FairRootManager *ioMan = FairRootManager::Instance();
   if (ioMan == nullptr) {
      FairLogger::GetLogger() -> Fatal(MESSAGE_ORIGIN, "Cannot find RootManager!");
      return kFATAL;
   }

   fInputEventArray = dynamic_cast<TClonesArray *>(ioMan->GetObject(fInputBranchName));
   if (fInputEventArray == nullptr) {
      FairLogger::GetLogger() -> Fatal(
                                 MESSAGE_ORIGIN, TString::Format("Cannot find TClonesArray in branch %s!", fInputBranchName).Data()
                                 );
      return kFATAL;
   }

   return kSUCCESS;
}

void STDataReductionTask::Exec(Option_t *opt)
{
   // Get raw event
   if (fInputEventArray->GetEntriesFast() == 0)
      return;
   fEvent = dynamic_cast<STRawEvent *>(fInputEventArray->At(0));

   // If we should skip this event mark bad and don't fill tree
   if (fReductionFunc()) {
      TString logString = 
         TString::Format("Keeping event %s at %s", fEvent->GetEventID(), FairRootManager::Instance()->GetEntryNr());
      FairLogger::GetLogger() -> Info(MESSAGE_ORIGIN, logString.Data());
   } else {

      TString logString = 
         TString::Format("Skipping event %s at %s", fEvent->GetEventID(), FairRootManager::Instance()->GetEntryNr());
      FairLogger::GetLogger() -> Info(MESSAGE_ORIGIN, logString.Data());

      FairRootManager *ioMan = FairRootManager::Instance();
      for (auto name : fOutputBranchs) {
         auto b = dynamic_cast<TClonesArray *>(ioMan->GetObject(name));
         if (fInputEventArray == nullptr)
            FairLogger::GetLogger() -> Fatal(MESSAGE_ORIGIN, 
                                             TString::Format("Cannot find branch %s!", name).Data()
                                            );

         auto e = dynamic_cast<STRawEvent *>(b->At(0));
         if (e == nullptr) {
            FairLogger::GetLogger() -> Error(MESSAGE_ORIGIN, 
                                             TString::Format("Not setting %s", name).Data()
                                            );
            continue;
         }
         e->SetIsGood(false);
      }
      FairRunAna::Instance()->MarkFill(false);
   }
}
