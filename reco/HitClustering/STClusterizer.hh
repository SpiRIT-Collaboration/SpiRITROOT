/**
 * @brief Virtual Clusterizer Class
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STCLUSTERIZER
#define STCLUSTERIZER

// SpiRITROOT classes
#include "STEvent.hh"
#include "STHit.hh"
#include "STHitCluster.hh"
#include "STDigiPar.hh"

// FairRoot classes
#include "FairRootManager.h"
#include "FairLogger.h"

// STL
#include <vector>

// ROOT classes
#include "TClonesArray.h"

class STClusterizer
{
  public:
    STClusterizer();
    virtual ~STClusterizer();

    virtual void Analyze(STEvent *eventIn, STEvent *eventOut = NULL) {}
    virtual void AnalyzeTrack(TClonesArray *trackArray, STEvent *eventOut) {}
    virtual void AnalyzeTrack(TClonesArray *trackArray, TClonesArray *clusterArray) {}

    void SetPrimaryVertex(TVector3 vertex);

    void SetProximityCut(Double_t x, Double_t y, Double_t z);
    void SetProximityCutInUnit(Double_t x, Double_t y, Double_t z);

    void SetSigmaCut(Double_t xw, Double_t yw, Double_t zw);
    void SetSigmaCutInUnit(Double_t xw, Double_t yw, Double_t zw);

    void SetEdgeCut(Double_t low, Double_t high);

    void SetClusteringOption(Int_t opt);

  protected:
    TVector3 fPrimaryVertex;  //!< primary vertex position

    Double_t fPadPlaneX;      //!< pad plane size x in mm
    Double_t fPadSizeX;       //!< pad size x in mm
    Double_t fPadSizeZ;       //!< pad size y in mm
    Int_t    fPadRows;        //!< number of total pad rows
    Int_t    fPadLayers;      //!< number of total pad layers
    Int_t    fNumTbs;         //!< the number of time buckets used in taking data
    Int_t    fTBTime;         //!< time duration of a time bucket in ns
    Double_t fDriftVelocity;  //!< drift velocity of electron in cm/us
    Double_t fMaxDriftLength; //!< maximum drift length in mm

    Double_t fYTb;            //!< 1 timebucket length;

    Double_t fXCut;
    Double_t fYCut;
    Double_t fZCut;
    Double_t fSigmaXCut;
    Double_t fSigmaYCut;
    Double_t fSigmaZCut;

    Double_t fXLowCut;
    Double_t fXHighCut;

    Int_t fNumHitsAtHead;

    Int_t fClusteringOption = 0;

    FairLogger *fLogger;      //!< logger


  ClassDef(STClusterizer, 1)
};

#endif
