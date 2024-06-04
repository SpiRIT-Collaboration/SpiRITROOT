#ifndef STBRANCH
#define STBRANCH

#include "STDataSubject.hh"


class TString;

namespace DataHandling {
/**
 * @brief Subject for the branch in the FairRoot tree.
 * @ingroup DataHandling
 */
class STBranch : public STSubject {
private:
   int fBranchId{-1};
   int fOldBranchId{-1};

public:
   /// Will notify on change
   void SetBranchName(const TString &name);
   /// Will notify on change
   void SetBranchId(const int id);

   TString GetBranchName() const;
   int GetBranchId() const { return fBranchId; }
   TString GetOldBranchName() const;
   int GetOldBranchId() const { return fOldBranchId; }
};
} // namespace DataHandling
#endif