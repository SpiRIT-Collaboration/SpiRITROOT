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
    void BuildTracks();
    void SortHits(std::vector<STLinearTrack*> *trackBuffer);
    void ChooseTracks();
    void MergeTracks(std::vector<STLinearTrack*> *trackBuffer);
    void CreateNewTrack(STHit *hit);

  private:
    Bool_t Correlate(STLinearTrack* track, STHit* hit);
    Bool_t CorrelateProximityXYZ(STLinearTrack* track, STHit* hit);
    Bool_t CorrelateProximity(STLinearTrack* track, STHit* hit);
    Bool_t CorrelatePerpPlane(STLinearTrack* track, STHit* hit);
    Bool_t CorrelatePerpLine(STLinearTrack* track, STHit* hit);
    Bool_t RMSTest(STLinearTrack* track, STHit* hit);

  private:
    STLinearTrackFitter* fLTFitter;

    std::vector<STHit*> *fHitBuffer;
    std::vector<STLinearTrack*> *fTrackBufferQueue;
    std::vector<STLinearTrack*> *fTrackBuffer;

    TVector3 fPrimaryVertex;

    Int_t fNumHHCompare;
    Int_t fMinNumHitFitLine;
    Int_t fMinNumHitFitPlane;

    Int_t fMinNumHitCut;

    Double_t fProxHHZCut;   ///< Cut for hit1 to hit2 distance in X
    Double_t fProxHHXCut;   ///< Cut for hit1 to hit2 distance in Z
    Double_t fProxHHYCut;   ///< Cut for hit1 to hit2 distance in Y
    Double_t fProxHHXZCut;  ///< Cut for hit1 to hit2 distance in XZ plane 
    Double_t fProxHHCut;    ///< Cut for hit1 to hit2 distance
    Double_t fProxHPCut;    ///< Cut for hit to (track) plane perpendicular distance
    Double_t fProxHTCut;    ///< Cut for hit to (track) line  perpendicular distance
    Double_t fRMSPlaneCut; ///< Cut for RMS in plane fitting for final track
    Double_t fRMSLineCut; ///< Cut for RMS in line  fitting for final track

  ClassDef(STLinearTrackFinder, 1)
};

#endif
