/**
 * @author JungWoo Lee
 *
 * @brief STCurveTrackingTask
 */

#ifndef STCURVETRACKINGTASK
#define STCURVETRACKINGTASK

// FairROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SpiRITROOT classes 
#include "STHit.hh"
#include "STDigiPar.hh"
#include "STCurveTrack.hh"
#include "STCurveTrackFinder.hh"

// ROOT classes
#include "TClonesArray.h"

// STD
#include <vector>

class STCurveTrackingTask : public FairTask
{
  public:
    STCurveTrackingTask();
    STCurveTrackingTask(Bool_t persistence);
    ~STCurveTrackingTask();

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    void SetPersistence(Bool_t value = kTRUE);

  private:
    Bool_t fIsPersistence;  ///< Persistence check variable

    FairLogger *fLogger; //!

    TClonesArray *fEventArray;
    TClonesArray *fTrackArray;

    STDigiPar *fPar; //!

    STCurveTrackFinder* fTrackFinder; //!

  ClassDef(STCurveTrackingTask, 1)
};

#endif
