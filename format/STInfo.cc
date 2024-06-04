#include "STInfo.hh"

#include <FairLogger.h> // for Logger, LOG

constexpr auto cRED = "\033[1;31m";
constexpr auto cYELLOW = "\033[1;33m";
constexpr auto cNORMAL = "\033[0m";
constexpr auto cGREEN = "\033[1;32m";
constexpr auto cBLUE = "\033[1;34m";

void STInfo::AddAugment(std::shared_ptr<STInfoBase> augment)
{
   if (augment == nullptr)
      return;

   AddAugment(augment, augment->GetDefaultName());
}

void STInfo::AddAugment(std::shared_ptr<STInfoBase> augment, std::string name)
{
   if (augment == nullptr)
      return;

   if (fInfoAugments.find(name) != fInfoAugments.end())
   FairLogger::GetLogger() -> Error(MESSAGE_ORIGIN, 
                                    TString::Format("STInfo augment %s already exists in this STInfo. If replacement is intentional, use STInfo::ReplaceAugment() instead.", name).Data()
                                    );

   ReplaceAugment(augment, name);
}

void STInfo::ReplaceAugment(std::shared_ptr<STInfoBase> augment)
{
   if (augment == nullptr)
      return;
   ReplaceAugment(augment, augment->GetDefaultName());
}
void STInfo::ReplaceAugment(std::shared_ptr<STInfoBase> augment, std::string name)
{
   if (augment == nullptr)
      return;
   fInfoAugments[name] = augment;
}

STInfo::BasePtr STInfo::GetAugment(std::string name)
{
   if (fInfoAugments.find(name) == fInfoAugments.end())
      return nullptr;
   else
      return fInfoAugments.at(name);
}
