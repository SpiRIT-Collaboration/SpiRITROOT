#ifndef STSOURCE
#define STSOURCE

// ROOT
#include "TString.h"

// FairRoot
#include "FairSource.h"

// SpiRITROOT
#include "STDecoderTask.hh"

class STSource : public FairSource {
  public:
    STSource();
    virtual ~STSource() {};

    Bool_t Init();
    Int_t ReadEvent(UInt_t);
    void Reset();

    void SetData(TString filename);
    void SetEventID(Long64_t eventid);

  private:
    TString fDataFile;
    STDecoderTask *fDecoder;
    Long64_t fEventID;

    Bool_t fIsInitialized;

  ClassDef(STSource, 1)
};

#endif
