#include "STBranch.hh"

#include <FairRootManager.h>
#include <FairRunAna.h>

#include <Rtypes.h>
#include <TString.h>
namespace DataHandling {

void STBranch::SetBranchName(const TString &name)
{
   SetBranchId(FairRootManager::Instance()->GetBranchId(name));
}

void STBranch::SetBranchId(const int id)
{
   if (fBranchId == id)
      return;

   fOldBranchId = fBranchId;
   fBranchId = id;
   Notify();
}

TString STBranch::GetBranchName() const
{
   return FairRootManager::Instance()->GetBranchName(fBranchId);
}

TString STBranch::GetOldBranchName() const
{
   return FairRootManager::Instance()->GetBranchName(fOldBranchId);
}

} // namespace DataHandling