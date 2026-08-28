#ifndef STCOPYTREETASK_H
#define STCOPYTREETASK_H

#include <FairTask.h>
#include "FairLogger.h"

#include <Rtypes.h> // for Option_t

#include <map>    // for map
#include <string> // for string
class TClonesArray;

/**
 * @brief Copy input FairRoot tree to the output.
 *
 * This MUST be the first task added to a run. It relies on some incredibly fragile logic in
 * FairRootManager involving the order in which branches are accessed in the input/output files.
 *
 *
 * Useful for applying additional cuts to data using AtDatareductionTask.
 * Will use the BranchList in the file source and duplicate the tree structurou in the output branch.
 */
class STCopyTreeTask : public FairTask {
private:
   std::map<std::string, TClonesArray *> fBranchMap; //< map<branchName, branchAddress>
    FairLogger *fLogger;                ///< FairLogger singleton

public:
   InitStatus Init() override;
   void Exec(Option_t *) override {}
};
#endif //#ifndef STCopyTREETASK_H
