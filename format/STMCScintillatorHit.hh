#ifndef STMCSCINTILLATORHIT
#define STMCSCINTILLATORHIT

#include "TObject.h"
#include "STMCPoint.hh"
#include <vector>

class STMCScintillatorHit : public TObject
{
  public:
    STMCScintillatorHit();
    virtual ~STMCScintillatorHit();

    STMCScintillatorHit(const STMCScintillatorHit&);


    /** modifier **/
    void SetDetectorID(Int_t id)   { fDetectorID = id; }
    void SetTotalEdep(Double_t de) { fTotalEdep = de; }
    void SetMaxZ(Int_t z)          { fMaxZ = z; }


    /** accessor **/
    Int_t GetDetectorID()   { return fDetectorID; }
    Double_t GetTotalEdep() { return fTotalEdep; }
    Int_t GetMaxZ()         { return fMaxZ; }

    void AddStep(STMCPoint *point);
    void AddStep(Double_t edep, STMCPoint *point);

  private:
    Int_t     fDetectorID;
    Double_t  fTotalEdep;
    Int_t     fMaxZ;
    std::vector<STMCPoint> fMCPointArray;

    ClassDef(STMCScintillatorHit,1);

};

#endif
