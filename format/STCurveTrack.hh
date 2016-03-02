/**
 * @brief SpiRITROOT curve track container 
 *
 * @author JungWoo Lee
 */

#ifndef STCURVETRACK
#define STCURVETRACK

#include "STLinearTrack.hh"
#include "STHit.hh"
#include "STHitCluster.hh"

#include "TObject.h"
#include "TVector3.h"

#include <vector>

class STCurveTrack : public STLinearTrack
{
  public:
    STCurveTrack();
    STCurveTrack(STCurveTrack *track);
    STCurveTrack(Int_t trackID, STHit* hit);
    virtual ~STCurveTrack();

    virtual void Clear(Option_t *option = "");

    void DeleteClusters();

    void RemoveHit(STHit *hit);
    Int_t GetNumHitsRemoved() const;

    void AddCluster(STHitCluster *cluster);
    Int_t GetNumClusters();

    std::vector<Int_t>  *GetClusterIDArray();
    std::vector<STHitCluster*> *GetClusterPointerArray();

    Int_t  GetClusterID(Int_t i);
    STHitCluster *GetCluster(Int_t i);

  private:
    Int_t fNumHitsRemoved;

    std::vector<Int_t> fClusterIDArray;
    std::vector<STHitCluster*> fClusterPtrArray; //!

  ClassDef(STCurveTrack, 2);
};

#endif
