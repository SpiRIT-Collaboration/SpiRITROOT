//-----------------------------------------------------------
// Description:
//   Clustering hits processed by PSATask
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//-----------------------------------------------------------

#ifndef _STCLUSTERINGTASK_H_
#define _STCLUSTERINGTASK_H_

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
    STHitClusteringTask();
    ~STHitClusteringTask();

    void SetPersistence(Bool_t value = kTRUE);
    void SetVerbose(Int_t value = 1);
    
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    STClusterizer* GetClusterizer();
    void SetClusterizerMode(Int_t mode);
    void SetParameters(Double_t *par);

  private:
    FairLogger *fLogger;           //!< FairLogger singleton

    Int_t fVerbose;                //!< Verbosity level

    STDigiPar *fPar;               //!< STDigiPar singleton

    Bool_t fIsPersistence;         //!< Persistancy setter

    STClusterizer *fClusterizer;  //!< Clusterizer pointer
    /**
     * Clusterizer Mode
     * - 0 : No Clusterizing
     * - 1 : STClusterizerScan
     */
    Int_t fClusterizerMode;

    Double_t fDriftLength;         //!< DriftLength parameter defined in ST.parameters.par [cm/ns]
    Int_t fYDivider;               //!< Space divider along y direction

    Double_t *fClusterizerPar;

    TClonesArray *fEventHArray;    //!< Array that is containing events having only hits
    TClonesArray *fEventHCArray;   //!< Array that will contain events having hits and hit clusters

    void FindCluster(vector<STHit> &slicedSpace, STEvent *event);
    STHit *FindLargestHitAndCloseHits(vector<STHit> &slicedSpace, STHit *centerHit, vector<Int_t> &clusteredHits);

  ClassDef(STHitClusteringTask, 1);
};

//! Class only for providing the sort criterion to vector used in STHitClusteringTask class
class STHitSortY
{
  public:
    STHitSortY() {}
    Bool_t operator()(STHit hitA, STHit hitB) { return (hitA.GetPosition().Y() < hitB.GetPosition().Y()); }
};

#endif
