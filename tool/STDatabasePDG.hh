#ifndef STDATABASEPDG
#define STDATABASEPDG

#include "TDatabasePDG.h"
#include <vector>

class STDatabasePDG
{
  public:
    static STDatabasePDG* Instance();
    STDatabasePDG();
    ~STDatabasePDG() {};

    void AddPDGCandidate(Int_t pdg);
    std::vector<Int_t> *GetPDGCandidateArray();

  private:
    std::vector<Int_t> fPDGCandidateArray;

    static STDatabasePDG *fInstance;
};

#endif
