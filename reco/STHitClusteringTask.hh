//-----------------------------------------------------------
// Description:
//   Clustering hits processed by PSATask
//
// Environment:
//   Software developed for the SPiRIT-TPC at RIKEN
//
// Author List:
//   Genie Jhang     Korea University     (original author)
//   JungWoo Lee     Korea University
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

    enum STClusterizerMode { kX, kScan, kScan2 };

    void SetPersistence(Bool_t value = kTRUE);
    void SetVerbose(Int_t value = 1);
    
    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    STClusterizer* GetClusterizer();
    void SetClusterizerMode(STClusterizerMode mode);

    void SetProximityCut(Double_t x, Double_t y, Double_t z);
    void SetSigmaCut(Double_t x, Double_t y, Double_t z);
    void SetEdgeCut(Double_t low, Double_t high);

  private:
    FairLogger *fLogger;           //!< FairLogger singleton
    Int_t fVerbose;                //!< Verbosity level
    STDigiPar *fPar;               //!< STDigiPar singleton
    Bool_t fIsPersistence;         //!< Persistancy setter

    STClusterizerMode fClusterizerMode;
    STClusterizer *fClusterizer;  //!< Clusterizer pointer

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

    Double_t fDriftLength;         //!< DriftLength parameter defined in ST.parameters.par [cm/ns]
    Int_t fYDivider;               //!< Space divider along y direction

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
