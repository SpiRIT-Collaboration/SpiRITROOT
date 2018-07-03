#ifndef STDETECTOR
#define STDETECTOR

#include "FairDetector.h"

#include "TVector3.h"
#include "TLorentzVector.h"

class STMCPoint;
class FairVolume;
class TClonesArray;

class STDetector: public FairDetector
{

  public:

    /**
     * Default Constructor with parameters.
     * @param Name Detector Name
     * @param Active kTRUE for active detectors (ProcessHits() will be called), kFALSE for inactive detectors
    */
    STDetector(const char* Name, Bool_t Active);

    /** Default Constructor **/
    STDetector();

    /** Default Destructor **/
    virtual ~STDetector();

    /** Initialization of the detector is done here **/
    virtual void  Initialize();

    /**
     * this method is called for each step during simulation
     * (see FairMCApplication::Stepping())
    */
    virtual Bool_t ProcessHits( FairVolume* v=0);

    /** Registers the produced collections in FAIRRootManager. **/
    virtual void Register();

    /** Gets the produced collections */
    virtual TClonesArray* GetCollection(Int_t iColl) const ;

    /** has to be called after each event to reset the containers **/
    virtual void Reset();

    /** Create the detector geometry */
    void ConstructGeometry();



    /**
     * This method is an example of how to add your own point
     * of type STMCPoint to the clones array
    */
    STMCPoint* AddHit(Int_t trackID, Int_t detID,
                             TVector3 pos, TVector3 mom,
                             Double_t time, Double_t length,
                             Double_t eLoss, Int_t pdg);

    /** Check the volume is sensitive or not **/
    virtual bool CheckIfSensitive(std::string name);

    /**
     * The following methods can be implemented if you need to make
     * any optional action in your detector during the transport.
    */

    virtual void CopyClones(TClonesArray* cl1, TClonesArray* cl2, Int_t offset) {;}
    virtual void SetSpecialPhysicsCuts() {;}
    virtual void EndOfEvent();
    virtual void FinishPrimary() {;}
    virtual void FinishRun() {;}
    virtual void BeginPrimary() {;}
    virtual void PostTrack() {;}
    virtual void PreTrack() {;}
    virtual void BeginEvent() {;}


  private:

    /** 
     * Track information to be stored until the track leaves the
     * active volume.
    */

    Int_t          fTrackID;           //!<  track index
    Int_t          fVolumeID;          //!<  volume id
    TLorentzVector fPos;               //!<  position at entrance
    TLorentzVector fMom;               //!<  momentum at entrance
    Double32_t     fTime;              //!<  time
    Double32_t     fLength;            //!<  length
    Double32_t     fELoss;             //!<  energy loss
	 Int_t 			 fPdg;					//!<  particle data group code

    /** Container for data points */
    TClonesArray*  fSTMCPointCollection;

    STDetector(const STDetector&);
    STDetector& operator=(const STDetector&);

    ClassDef(STDetector,1)
};

#endif
