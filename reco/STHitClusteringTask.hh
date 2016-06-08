/**
 * @brief Clustering hits processed by PSATask
 *
 * @author Genie Jhang (Korea University), original author
 * @author JungWoo Lee (Korea University)
 */

#ifndef STCLUSTERINGTASK_HH
#define STCLUSTERINGTASK_HH

// SpiRITROOT classes
#include "STEvent.hh"
#include "STHit.hh"
#include "STHitCluster.hh"
#include "STDigiPar.hh"
#include "STClusterizer.hh"

// FairROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// ROOT classes
#include "TClonesArray.h"
#include "TVector3.h"

// STL
#include <vector>

using std::vector;

class STHitClusteringTask : public FairTask
{
  public:
    enum STClusterizerMode { kX, kScan, kScan2, kLT, kCT };

    STHitClusteringTask();
    STHitClusteringTask(Bool_t persistence, STClusterizerMode mode = STHitClusteringTask::kCT);
    ~STHitClusteringTask();

    void SetVerbose(Int_t value = 1);
    
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    void SetPersistence(Bool_t value = kTRUE);

    STClusterizer* GetClusterizer();
    void SetClusterizerMode(STClusterizerMode mode);
    void SetClusterByLength(Bool_t flag);

    void SetProximityCut(Double_t x, Double_t y, Double_t z);
    void SetSigmaCut(Double_t x, Double_t y, Double_t z);
    void SetEdgeCut(Double_t low, Double_t high);

  private:
    Bool_t fIsPersistence;  ///< Persistence check variable

    TClonesArray *fEventArray;    
    TClonesArray *fTrackArray;    

    STClusterizer *fClusterizer;   //!< Clusterizer pointer
    STClusterizerMode fClusterizerMode;

      Bool_t fSetProxCut;
    Double_t fXCut;
    Double_t fYCut;
    Double_t fZCut;

      Bool_t fSetSigmaCut;
    Double_t fSigmaXCut;
    Double_t fSigmaYCut;
    Double_t fSigmaZCut;

      Bool_t fSetEdgeCut;
    Double_t fXLowCut;
    Double_t fXHighCut;

    STDigiPar *fPar;       ///< STDigiPar singleton
    Double_t fDriftLength; ///< Drift length [mm]

    FairLogger *fLogger;   //!< FairLogger singleton

    Bool_t fClusterByLength = kFALSE;

  ClassDef(STHitClusteringTask, 2);
};

#endif
