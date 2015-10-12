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

    Double_t fProxHHCut;
    Double_t fProxHTCut;
    Double_t fRMSCut;

  ClassDef(STLinearTrackFinder, 1)
};

#endif
