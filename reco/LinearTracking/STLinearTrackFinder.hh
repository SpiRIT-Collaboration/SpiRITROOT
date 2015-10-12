/**
 * @author JungWoo Lee
 */

#ifndef STLINEARTRACKFINDER
#define STLINEARTRACKFINDER

#include "STEvent.hh"
#include "STHit.hh"
#include "STLinearTrack.hh"
#include "STLinearTrackFitter.hh"

#include <vector>

class STLinearTrackFinder
{
  public:
    STLinearTrackFinder();
    ~STLinearTrackFinder();

    void BuildTracks(STEvent* event, 
                     std::vector<STLinearTrack*> *trackBuffer);

  private:
    void CreateNewTrack(STHit *hit);

  private:
    Bool_t PerpDistCorrelator(STLinearTrack* track, STHit* hit);
    Bool_t ProximityCorrelator(STLinearTrack* track, STHit* hit);
    Bool_t QualityTest(STLinearTrack* track, STHit* hit);

    STLinearTrackFitter* fLTFitter;

    std::vector<STLinearTrack*> *fTrackBuffer;
    std::vector<STHit*> *fHitBuffer;

    TVector3 fPrimaryVertex;

    Double_t fProxCut;
    Double_t fRMSCut;

  ClassDef(STLinearTrackFinder, 1)
};

class STLTFHitSortPhi {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2) 
    { return hit1 -> GetPosition().Phi() < hit1 -> GetPosition().Phi(); }
};

class STLTFHitSortTheta {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().Theta() < hit1 -> GetPosition().Theta(); }
};

class STLTFHitSortR {
  public:
    Bool_t operator() (STHit* hit1, STHit* hit2)
    { return hit1 -> GetPosition().Mag() < hit1 -> GetPosition().Mag(); }
};

#endif
