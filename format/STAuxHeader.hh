#ifndef STAUXHEADER
#define STAUXHEADER

#include "TNamed.h"

#include <iostream>

class STAuxHeader : public TNamed
{
  public:
    STAuxHeader();
    virtual ~STAuxHeader() {};

    virtual void Clear(Option_t *option = "");
    //virtual void Print(Option_t *option = "") const;

    Int_t GetBdcID() { return fBdcID; }
    Int_t GetTpcEventNum() { return fTpcEventNum; }
    void SetBdcID(Int_t val) { fBdcID = val; }
    void SetTpcEventNum(Int_t val) { fTpcEventNum = val; }

    ULong_t GetTpcTime() { return fTpcTime; }
    void SetTpcTime(ULong64_t val) { fTpcTime = val; }

  private:
    Int_t fTpcEventNum; ///< Event Number for the TPC event
    Int_t fBdcID;       ///< Event ID for the corresponding BDC event

    ULong_t fTpcTime; ///< Timestamp for this event from the GET DAQ

  ClassDef(STAuxHeader, 1)
};

#endif
