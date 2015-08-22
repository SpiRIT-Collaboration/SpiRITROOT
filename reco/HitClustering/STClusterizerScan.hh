#ifndef STCLUSTERIZERSCAN
#define STCLUSTERIZERSCAN

#include "STClusterizer.hh"

#include "TVector3.h"
#include "TVector2.h"

//#define DEBUG

#include "TCanvas.h"
#include "TH2D.h"
#include "TGraph.h"
#include "TGraphErrors.h"

class STClusterizerScan : public STClusterizer
{
  public:
    STClusterizerScan();
    ~STClusterizerScan();

    void Analyze(STEvent* eventH, STEvent* eventHC);
    void SetParameters(Double_t *par);

    void SetPrimaryVertex(TVector3 vertex);

    void SetZCut(Double_t zCut);
    void SetZCutPadUnit(Double_t zCut);
    void SetVerticalCut(Double_t vCut);
    void SetVerticalCutTbUnit(Double_t vCut);
    void SetHorizontalCut(Double_t hCut);
    void SetHorizontalCutPadUnit(Double_t hCut);

    void SetSigmaXCut(Double_t xCut);
    void SetSigmaXCutPadUnit(Double_t xCut);
    void SetSigmaYCut(Double_t yCut);
    void SetSigmaYCutTbUnit(Double_t yCut);

  private:
    std::vector<STHit *> *fHitArray;
    std::vector<STHitCluster *> *fHitClusterArray;

    TVector3 fPrimaryVertex; // primary vertex position

    /**
     * Correlator between hit and cluster.
     * Returns true if hit should be added to the cluster.
     */
    Bool_t CorrelateHC(STHit* hit, STHitCluster* cluster);

    Double_t fVerticalCut;
    Double_t fHorizontalCut;

    void AddClusterToEvent(STEvent* eventHC, STHitCluster* cluster);

    STHitCluster* fClusterTemp;

    TCanvas* fCvs;
    TH2D*    fFrame;
    TGraph*  fGraphHit;
    TGraph*  fGraphAddedHit;
    TGraphErrors* fGraphCurrentCluster;
    TGraphErrors* fGraphCluster;
    TGraphErrors* fGraphFinishedCluster;

  ClassDef(STClusterizerScan, 1)
};


class STHitSortR
{
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return (hit1 -> GetPosition()).Mag() < (hit2 -> GetPosition()).Mag(); }
};

#endif
