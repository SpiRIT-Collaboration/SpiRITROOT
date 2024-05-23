#include "STMacroTask.hh"

ClassImp(STMacroTask);

InitStatus STMacroTask::Init()
{
   for (auto func : fInitFunctions)
      func();

   return kSUCCESS;
}

void STMacroTask::Exec(Option_t *option)
{
   for (auto func : fFunctions)
      func();
}
