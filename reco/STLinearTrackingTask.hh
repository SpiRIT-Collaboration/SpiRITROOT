/**
 * @author JungWoo Lee
 */

#ifndef STLINEARTRACKINGTASK
#define STLINEARTRACKINGTASK

// FairROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SpiRITROOT classes 
#include "STHit.hh"
#include "STDigiPar.hh"
#include "STLinearTrack.hh"
#include "STLinearTrackFinder.hh"

// ROOT classes
#include "TClonesArray.h"

// STD
#include <vector>

class STLinearTrackingTask : public FairTask
{
  public:
    STLinearTrackingTask();
    ~STLinearTrackingTask();

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

  private:
    FairLogger *fLogger; //!

    TClonesArray *fEventArray;
    TClonesArray *fTrackArray;
    std::vector<STLinearTrack*> *fTrackBuffer;

    STDigiPar *fPar; //!

    STLinearTrackFinder* fLinearTrackFinder; //!

  ClassDef(STLinearTrackingTask, 1);
};

#endif
