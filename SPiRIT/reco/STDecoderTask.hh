//-----------------------------------------------------------
// Description:
//   Converting GRAW file to tree structure to make it easy
//   to access the data
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef _STDECODERTASK_H_
#define _STDECODERTASK_H_

// FAIRROOT classes
#include "FairTask.h"

// SPiRITROOT classes
#include "STCore.hh"

#include "STCore.hh"
#include "STMap.hh"
#include "STPedestal.hh"

#include "STDigiPar.hh"

// ROOT classes

class STDecoderTask : public FairTask {
  public:
    STDecoderTask();
    ~STDecoderTask();

    void SetNumTbs(Int_t numTbs);
    void SetGraw(Char_t *filename);

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    STCore *fDecoder;

    Char_t *fGrawFile;
    Int_t fNumTbs;

    STDigiPar *fPar;

  ClassDef(STDecoderTask, 1);
};

#endif
