#ifndef STAUXHEADERTASK_H
#define STAUXHEADERTASK_H

#include <FairTask.h>
#include "FairLogger.h"

#include "STAuxHeader.hh"

#include "TString.h"

class STAuxHeaderTask : public FairTask {
private:
    STAuxHeader *fAuxHeader = nullptr;
    STAuxHeader *fInputAuxHeader = nullptr;

    TString fAuxHeaderBranch = "STAuxHeader";
    TString fInputAuxHeaderBranch = "";
    Bool_t fIsPersistence = kFALSE;              ///< Persistence check variable

    Int_t fEventID;

public:

    STAuxHeaderTask();
    ~STAuxHeaderTask();

    virtual InitStatus Init();
    virtual void Exec(Option_t *opt);

    void SetPersistence(Bool_t value = kTRUE) { fIsPersistence = value; }
    void SetAuxBranch(TString name = "STAuxHeader") { fAuxHeaderBranch = name; }
    void SetInputBranch(TString name = "MCAuxHeader") { fInputAuxHeaderBranch = name; }

    void SetEventNum(Int_t value);
};

#endif
