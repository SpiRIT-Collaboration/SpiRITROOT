#ifndef STCLUSTERIZERSCAN2
#define STCLUSTERIZERSCAN2

#include "STClusterizer.hh"

#include "STHitClusterRich.hh"

#include "TMath.h"
#include "TVector3.h"

class STClusterizerScan2 : public STClusterizer
{
  public:
    STClusterizerScan2();
    ~STClusterizerScan2();

    void Analyze(STEvent* eventIn, STEvent* eventOut = NULL);
    void AnalyzeTrack(TClonesArray* trackArray, STEvent* eventOut);
    void AnalyzeHitArray(STEvent* eventOut);

  private:
    std::vector<STHit *> *fHitArray;
    std::vector<STHitClusterRich *> *fHitClusterFinalArray;
    std::vector<STHitClusterRich *> *fHitClusterTempArray;

    /**
     * Correlator between hit and cluster.
     * Returns true if hit should be added to the cluster.
     */
    Bool_t CorrelateHC(STHit*, STHitClusterRich*);
    Bool_t CorrelateCC(STEvent*, STHitClusterRich*, STHitClusterRich*);
    void   MergeCC(STEvent*, STHitClusterRich*, STHitClusterRich*);

    void AddClusterToEvent(STEvent*, STHitClusterRich*);

    STHitClusterRich* fClusterTemp;

  ClassDef(STClusterizerScan2, 1)
};

#endif
