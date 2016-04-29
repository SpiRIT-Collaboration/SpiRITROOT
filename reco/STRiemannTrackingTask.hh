/**
 * @brief   Pattern recognition (track finding) 
 *          in the SPiRIT-TPC using the Riemann track fit.
 *
 * @author  Sebastian Neubert (TUM) -- original author for FOPIROOT
 * @author  Johannes Rauch -- FOPIROOT
 * @author  Felix Boehmer  -- FOPIROOT
 * @author  Genie Jhang (Korea University) -- implementation for SpiRITROOT
 * @author  JungWoo Lee (Korea University) -- implementation for SpiRITROOT
 */

#ifndef STRIEMANNTRACKINGTASK
#define STRIEMANNTRACKINGTASK

// FairROOT classes
#include "FairTask.h"
#include "FairLogger.h"

// SpiRITROOT classes 
#include "STRiemannTrack.hh"
#include "STCurveTrack.hh"
#include "STHit.hh"
#include "STRiemannSort.hh"
#include "STRiemannTrackFinder.hh"
#include "STDigiPar.hh"

// ROOT classes
#include "TClonesArray.h"

class STRiemannTrackingTask : public FairTask
{
  public:
    STRiemannTrackingTask();
    STRiemannTrackingTask(Bool_t persistence);
    ~STRiemannTrackingTask();

    /** Set sorting parameters
     *
     * @param sortingMode
     *        false: sort only according to sorting,
     *        true: use internal sorting when adding hits to trackcands
     *
     * @param sorting 
     *       -1: no sorting, 
     *        0: sort Hits by X, 
     *        1: Y, 
     *        2: Z, 
     *        3: R,
     *        4: distance to interaction point,
     *        5: Phi, -5: -Phi
     *
     * @param interactionZ
     *        set if you use sorting = 4.
     */
    void SetSortingParameters(Bool_t sortingMode = kTRUE,  
                               Int_t sorting = STRiemannSort::kSortY, 
                            Double_t interactionZ = 0);

    /** Set multi step parameters
     *
     * @param doMultistep
     *        if true, do a multistep approach:
     *        1. find steep tracks (presort hits along z)
     *        2. find circle tracks  (presort hits by angle)
     *        3. find all other tracks (presort hits by decreasing radius)
     *
     * @param minHitsZ    minimum number of hits for a track to be found in step 1.
     * @param minHitsR    minimum number of hits for a track to be found in step 1.
     * @param minHitsPhi  minimum number of hits for a track to be found in step 2.
     */
    void SetMultistepParameters(Bool_t doMultistep,
                                UInt_t minHitsZ = 10,
                                UInt_t minHitsR = 10,
                                UInt_t minHitsPhi = 10);

    /** Set track finder parameters
     *
     * @param proxcut          proximity cut in 3D
     * @param hexlicut         distance to helix cut
     * @param minpointsforfit  minimum number of hits in track before a helix is fitted
     * @param zStretch         stretch proximity cut in z direction
     */
    void SetTrkFinderParameters(Double_t proxcut,
                                Double_t helixcut,
                                  UInt_t minpointsforfit,
                                Double_t zStretch = 1.);

    /// max RMS of distances to helix for a track to be written out
    void SetMaxRMS(Double_t value);

    /// merge tracklets
    void SetMergeTracks(Bool_t mergeTracks = kTRUE);

    /// merge curlers
    void SetMergeCurlers(Bool_t mergeCurlers = kTRUE, Double_t blowUp = 5.);

    /** Set track merger parameters
     *
     * @param TTproxcut   proximity cut in 3D
     * @param TTdipcut    cut on difference of dip angles of tracklets
     * @param TThelixcut  distance of the two helices
     * @param TTplanecut  cut on rms of distances of the riemann hits to 
     *                    intersection of the plane with the sphere of a combined fit
     */
    void SetTrkMergerParameters(Double_t TTproxcut,
                                Double_t TTdipcut,
                                Double_t TThelixcut,
                                Double_t TTplanecut);

    /// blow up factor of the riemann sphere
    void SetRiemannScale(Double_t riemannscale = 8.7);
    
    /// skip and remove hits which would match to more than one track at highest correlator level
    void SkipCrossingAreas(Bool_t value = kTRUE);

    void UseDefaultParameterSet();
    void UsePANDAParameterSet();
    void UseFOPIParameterSet();

    virtual InitStatus Init();
    virtual void SetParContainers();
    virtual void Exec(Option_t *opt);

    void SetPersistence(Bool_t value = kTRUE);

    void SetSubTaskTracks(Int_t id1, Int_t id2);


  private:
    Bool_t fIsPersistence;  ///< Persistence check variable

    FairLogger *fLogger;

    void BuildTracks(STRiemannTrackFinder *trackfinder,
                     std::vector<STHit *> *hitBuffer,
                     std::vector<STRiemannTrack*> *TrackletList,
                     Int_t sorting,
                     UInt_t minHits,
                     Double_t maxRMS,
                     Bool_t skipCrossingAreas = kTRUE,
                     Bool_t skipAndDelete = kTRUE);

    void MergeTracks();
    void MergeCurlers();

    TClonesArray *fEventHCMArray;
    TClonesArray *fRiemannTrackArray;
    TClonesArray *fRiemannHitArray;
    TClonesArray *fPreTrackArray;

    STDigiPar *fPar;

    std::vector<STHit *> *fHitBuffer;
    std::vector<STHit *> *fHitBufferTemp;
    std::vector<STRiemannTrack *> fRiemannList;

    STRiemannTrackFinder *fTrackFinder;
    STRiemannTrackFinder *fTrackFinderCurl;

    Bool_t fSortingMode;
    Int_t fSorting;
    Double_t fInteractionZ;
    
    Double_t fRiemannScale;

    UInt_t fMinPoints;
    Double_t fProxCut;
    Double_t fProxZStretch;
    Double_t fHelixCut;

    Bool_t fMergeTracks;
    Double_t fTTProxCut;
    Double_t fTTDipCut;
    Double_t fTTHelixCut;
    Double_t fTTPlaneCut;

    Bool_t fMergeCurlers;
    Double_t fBlowUp;

    Bool_t fSkipCrossingAreas;

    Bool_t fDoMultiStep;
    UInt_t fMinHitsZ;
    UInt_t fMinHitsR;
    UInt_t fMinHitsPhi;

    Double_t fMaxRMS;


  ClassDef(STRiemannTrackingTask, 1);
};

#endif
