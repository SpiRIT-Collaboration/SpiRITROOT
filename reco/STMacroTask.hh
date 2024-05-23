#ifndef STMACROTASK_H
#define STMACROTASK_H

#include <FairTask.h>

#include <Rtypes.h> // for ClassDef

#include <functional> // for function
#include <vector>

class TClass;

/**
 * A class to run a supplied function or list of functions, in the order provided, when the task is initialized or
 * executed. This class is useful if there are user written functions stored in a header file that may be used in
 * multiple macros. This class is also useful if wanting to use the data handling AtTabInfo classes from the viewer in
 * general analysis.
 */
class STMacroTask : public FairTask {
private:
   std::vector<std::function<void()>> fInitFunctions;
   std::vector<std::function<void()>> fFunctions;

public:
   STMacroTask() = default;
   ~STMacroTask() = default;

   InitStatus Init();
   void Exec(Option_t *option);

   /**
    * Adds a user defined function to the init function list. Functions are executed in the order they were added when
    * the task is initialized.
    */
   void AddInitFunction(std::function<void()> function) { fInitFunctions.push_back(function); }

   /**
    * Adds a user defined function to the function list. Functions are executed in the order they were added when the
    * task is executed.
    */
   void AddFunction(std::function<void()> function) { fFunctions.push_back(function); }

   ClassDef(STMacroTask, 1);
};

#endif
