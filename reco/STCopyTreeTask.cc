#include "STCopyTreeTask.hh"

#include <FairLogger.h>      // for LOG
#include <FairRootManager.h> // for FairRootManager

#include <TClonesArray.h>
#include <TList.h>   // for TList
#include <TObject.h> // for TObject
#include <TString.h> // for TString

InitStatus STCopyTreeTask::Init()
{
   fLogger = FairLogger::GetLogger();
   FairRootManager *ioMan = FairRootManager::Instance();
   if (ioMan == nullptr) {
      fLogger -> Fatal(MESSAGE_ORIGIN, "Cannot find RootManager!");
      return kFATAL;
   }

   // Loop through the source branch list and grab every branch.
   for (int i = 0; i < ioMan->GetBranchNameList()->GetSize(); i++) {

      auto branchName = ioMan->GetBranchName(i);
      auto branchArray = dynamic_cast<TClonesArray *>(ioMan->GetObject(branchName));
      if (branchArray == nullptr)
         continue;

      if (ioMan->CheckBranch(branchName) == 1)
         ioMan->Register(branchName, "AtTPC", branchArray, true);
   }
   return kSUCCESS;
}
