/**
 * @brief SPiRIT PHITS Event Generator
 * @author Genie Jhang (Korea Univ.)
 */

#ifndef STPHITSEVENTGENERATOR
#define STPHITSEVENTGENERATOR

// FairRoot
#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"
#include "FairLogger.h"

// ROOT
#include "TFile.h"
#include "TTree.h"
#include "TClonesArray.h"

class STPHITSEventGenerator : public FairGenerator
{
  public :

    /** Constructor **/
    STPHITSEventGenerator();
    STPHITSEventGenerator(const STPHITSEventGenerator&);
    STPHITSEventGenerator& operator=(const STPHITSEventGenerator&);

    /** Destructor. **/
    virtual ~STPHITSEventGenerator() {}

    /** Set PHITS ROOT data file (with tree name) **/
    void SetData(TString filename, TString treename = "tree");

    /** Set primary vertex position **/
    void SetPrimaryVertex(Double_t x, Double_t y, Double_t z);

    /** Get the number of events in PHITS data file **/
    Long64_t GetNumEvents();

    /** Get current event number **/
    Long64_t GetCurrentEvent() ;

    void SetSingleTrackEvent();

    void SetEvent(Int_t event);

    Int_t GetNumTracks(Int_t event);

    /** 
     * Abstract method in base class.
     * Reads event from the input file and pushes the tracks onto the stack. 
     * @param primGen  pointer to the FairPrimaryGenerator
     **/
    virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);

  private :
           TFile *fDataFile;       ///< Data file
           TTree *fDataTree;       ///< Data tree
    TClonesArray *fEventArray;     ///< Event array
        Double_t  fVx;             ///< x position of primary vertex
        Double_t  fVy;             ///< x position of primary vertex
        Double_t  fVz;             ///< x position of primary vertex
        Long64_t  fNumEvents;      ///< Total number of events
        Long64_t  fCurrentEvent;   ///< Current event number

          Bool_t  fSingleTrackEventFlag;
        Long64_t  fCurrentParticle;

    Int_t kfToPDG(Long64_t kfCode);

  ClassDef(STPHITSEventGenerator,1);
};

class Particle : public TObject
{
  public:
    Particle() { kf=0; }
    virtual ~Particle() {}
   
    Int_t kf; // kf code, see page 14 of http://phits.jaea.go.jp/manual/manualJ-phits.pdf  
    Int_t q; // charge
    Int_t A; // mass number
    Double_t m; // rest mass [MeV]
    Double_t ke; // kinetic energy [MeV/c2]
    Double_t pos[3]; // position [mm]
    Double_t mom[3]; // momentum in MeV for colpart, unit vec for part
   
    Double_t brho; // brho
    Double_t xang; // atan(px/pz)
    Double_t yang; // atan(py/pz)
   
  ClassDef(Particle, 1);
};

#endif
