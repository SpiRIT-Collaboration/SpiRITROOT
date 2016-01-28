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
#include "STVTrackFinder.hh"
#include "STLinearTrackFinder.hh"
#include "STLinearTrackFinder2.hh"

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

    void SetPersistence(Bool_t value = kTRUE);

    STVTrackFinder* GetTrackFinder();

  private:
    Bool_t fIsPersistence;  ///< Persistence check variable

    FairLogger *fLogger; //!

    TClonesArray *fEventArray;
    TClonesArray *fTrackArray;
    std::vector<STLinearTrack*> *fTrackBuffer;

    STDigiPar *fPar; //!

    STVTrackFinder* fTrackFinder; //!

  ClassDef(STLinearTrackingTask, 1);
};

#endif
