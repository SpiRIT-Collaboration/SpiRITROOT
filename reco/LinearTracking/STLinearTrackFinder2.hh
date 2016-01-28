/**
 * @author JungWoo Lee
 */

#ifndef STLINEARTRACKFINDER2
#define STLINEARTRACKFINDER2

#include "STVTrackFinder.hh"

#include <vector>
#include <iostream>
#include <map>
#include <string>


class STLinearTrackFinder2 : public STVTrackFinder
{
  public:
    STLinearTrackFinder2();
    ~STLinearTrackFinder2() {}

    /// Correlation status
    enum STCoStatus 
    { 
      /// Correlated track is candidate for merging
      kCANDIDATE,
      /// Correlated track is not candidate of merging. new track will be created.
      kFAIL,
      /// Although it failed, information was not enough, so put it back into queue.
      kQUEUE
    };

    friend std::ostream& operator<<(std::ostream& out, const STCoStatus value)
    {
      static std::map<STCoStatus, std::string> strings;
      if (strings.size() == 0)
      {
#define INSERT_ELEMENT(p) strings[p] = #p
        INSERT_ELEMENT(kCANDIDATE);     
        INSERT_ELEMENT(kFAIL);     
        INSERT_ELEMENT(kQUEUE);             
#undef INSERT_ELEMENT
      }   
      return out << strings[value];
    }

    /// Build tracks from event, add tracks in trackBuffer
    void BuildTracks(STEvent* event, std::vector<STLinearTrack*> *trackBuffer);

    void SetCutStretch(Double_t stretchXZ, Double_t stretchY);

  private: // methods
    void NewTrack(STHit *hit);
    void BuildTracksFromQueue(Int_t numIter, 
                              Int_t idxHitLimit = -1, 
                              Bool_t emptyQueue = kFALSE);
    void MergeTracks(std::vector<STLinearTrack*> *trackBuffer);
    void SortHits(std::vector<STLinearTrack*> *trackBuffer);
    void SelectTracks(std::vector<STLinearTrack*> *trackBufferIn,
                      std::vector<STLinearTrack*> *trackBufferOut,
                      Int_t numHitsCut = -1);

    /// hit/track correlator master
    STCoStatus CorrelateHT(STLinearTrack* track, STHit* hit, Double_t &rms);

    /// hit/hit-in-track proximity correlator
    STCoStatus CorrelateHTProximity(STLinearTrack* track, STHit* hit);
    /// hit/track-plane perpendicular distance correlator 
    STCoStatus CorrelateHTPerpPlane(STLinearTrack* track, STHit* hit);
    /// hit/track-line perpendicular distance correlator 
    STCoStatus CorrelateHTPerpLine(STLinearTrack* track, STHit* hit);
    /// RMS test with hit/track merged
    STCoStatus RMSTestHT(STLinearTrack* track, STHit* hit, Double_t &rms);


    /// track/track correlators master
    STCoStatus CorrelateTT(STLinearTrack* trk1, STLinearTrack* trk2, Double_t &rms);

    /// track/track difference correlator between direction and normal vectors
    STCoStatus CorrelateTTGeo(STLinearTrack* trk1, STLinearTrack* trk2);
    /// track/track perpendicular distance from hit-in-track to track
    STCoStatus CorrelateTTPerp(STLinearTrack* trk1, STLinearTrack* trk2);
    /// RMS test with track/track merged
    STCoStatus RMSTestTT(STLinearTrack* trk1, STLinearTrack* trk2, Double_t &rms);

  private:
    STLinearTrackFitter *fLTFitter;

    std::vector<STHit*> *fHitQueue;
    std::vector<STLinearTrack*> *fTrackQueue;
    std::vector<STLinearTrack*> *fTrackBuffer;

    /// Primary vertex of the event
    ///
    /// TODO: 
    ///   We may need find primary vertex of the event before 
    ///   building tracks rather than fixed number. Now it is
    ///   fixed, this may cause problem later.
    TVector3 fPrimaryVertex;

    Double_t fStretchXZ;
    Double_t fStretchY;
  
  public:

  ClassDef(STLinearTrackFinder2, 1)
};

#endif
