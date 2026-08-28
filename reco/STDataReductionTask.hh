#ifndef STDATAREDUCTIONTSAK_H
#define STDATAREDUCTIONTSAK_H

#include <FairTask.h>

#include <Rtypes.h>
#include <TString.h>

#include <algorithm>  // for max
#include <functional> // for function
#include <utility>    // for move
#include <vector>     // for vector

class STRawEvent;
class TClonesArray;
class TBuffer;
class TClass;
class TMemberInspector;

/**
 * Task to reduce the data. It takes a function pointer of the type:
 * bool func(AtRawEvent*)
 * If the function returns true, it keeps the event. Otherwise it skips writing
 * it to disk and sets AtRawEvent.fIsGood as false.
 *
 */
class STDataReductionTask : public FairTask {
private:
   using ReductionFunction = std::function<bool()>;

   ReductionFunction fReductionFunc{nullptr};

   TClonesArray *fInputEventArray{nullptr}; //< AtRawEvent branch object
   TString fInputBranchName{"STRawEvent"};  //< Name of AtRawEvent branch to mark as good/not good
   STRawEvent *fEvent;
   std::vector<TString> fOutputBranchs;

public:
   STDataReductionTask() = default;
   ~STDataReductionTask() = default;

   void SetReductionFunction(ReductionFunction func) { fReductionFunc = std::move(func); }
   void SetReductionFunction(std::function<bool(STRawEvent *)> func);

   /// This will cast whatever is in the input array to type T and then call the passed function
   template <class T>
   void SetReductionFunction(std::function<bool(T *)> func)
   {
      fReductionFunc = [this, func]() { return func(dynamic_cast<T *>(fEvent)); };
   }
   void SetInputBranch(TString inputBranch)
   {
      fInputBranchName = inputBranch;
      fOutputBranchs.push_back(inputBranch);
   }
   void SetOutputBranch(TString outputBranch) { fOutputBranchs.push_back(outputBranch); }

   virtual InitStatus Init() override;
   virtual void Exec(Option_t *opt) override;

   ClassDefOverride(STDataReductionTask, 2)
};

#endif //#ifndef ATDATAREDUCTIONTSAK_H
