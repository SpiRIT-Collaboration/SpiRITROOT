#pragma once

#include "STClusterizer.hh"

#include "TVector3.h"

class STClusterizerScan : public STClusterizer
{
  public:
    STClusterizerScan();
    ~STClusterizerScan();

    void Analyze(STEvent* eventH, STEvent* eventHC);

    void SetPrimaryVertex(TVector3 vertex);
    void SetYCut(Double_t yCut);
    void SetYCutTbUnit(Double_t tbCut);
    void SetPerpCut(Double_t perpCut);
    void SetPerpCutPadUnit(Double_t xCut, Double_t zCut);

  private:
    std::vector<STHit *> *fHitArray;
    std::vector<STHitCluster *> *fHitClusterArray;

    Double_t fYTb; // 1 timebucket length;

    TVector3 fPrimaryVertex; // primary vertex position

    /**
     * Correlator between hit and cluster.
     * Returns true if hit should be added to the cluster.
     */
    Bool_t CorrelateHC(STHit* hit, STHitCluster* cluster);

    Double_t fXCut; 
    Double_t fYCut;
    Double_t fZCut; 
    Double_t fPerpCut; 
    Double_t fRCut;

    void SetAngle(STHit* hit);
    void AddClusterToEvent(STEvent* eventHC, STHitCluster* cluster);

  ClassDef(STClusterizerScan, 1)
};


class STHitSortR
{
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return (hit1 -> GetPosition()).Mag() < (hit2 -> GetPosition()).Mag(); }
};
