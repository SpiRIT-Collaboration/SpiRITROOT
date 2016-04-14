#ifndef GETMUTANTFRAME
#define GETMUTANTFRAME

#include "GETHeaderBase.hh"

#include <bitset>

class GETMutantFrame : public GETHeaderBase {
  public:
    GETMutantFrame();

    ULong64_t GetTimestamp();
       UInt_t GetEventNumber();
       UInt_t GetTriggerInfo();
       UInt_t GetMultiplicity(Int_t id);
       UInt_t GetEventCounter(Int_t id);
       UInt_t GetScaler(Int_t id);
       UInt_t GetD2PTime();

    ULong64_t GetFrameSkip();
    ULong64_t GetHeaderSkip();

    void Clear(Option_t * = "");
    void Read(ifstream &Stream);

    void Print();

  private:
    uint8_t fTimestamp[6];
    uint8_t fEventNumber[4];
    uint8_t fTriggerInfo[2];
    uint8_t fMultiplicity[4];
    uint8_t fEventCounter[16];
    uint8_t fScaler[20];
    /*
    uint8_t fMultiplicityA[2];
    uint8_t fMultiplicityB[2];
    uint8_t fL0EventCounter[2];
    uint8_t fL1AEventCounter[2];
    uint8_t fL1BEventCounter[2];
    uint8_t fL2EventCounter[2];
    uint8_t fScaler1[2];
    uint8_t fScaler2[2];
    uint8_t fScaler3[2];
    uint8_t fScaler4[2];
    uint8_t fScaler5[2];
    */
    uint8_t fD2PTime[4];

  ClassDef(GETMutantFrame, 1)
};

#endif
