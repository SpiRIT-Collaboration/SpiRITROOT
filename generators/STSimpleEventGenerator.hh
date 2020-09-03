/**
 * @brief SPiRIT Simple Event Generator
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STSIMPLEEVENTGENERATOR
#define STSIMPLEEVENTGENERATOR

#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"
#include "FairLogger.h"

#include "TFile.h"
#include "TROOT.h"
#include "TNtuple.h"
#include "TVector3.h"
#include "TRandom3.h"

#include <fstream>

class STSimpleEventGenerator : public FairGenerator
{
  public :

    /** Default constructor without arguments. Do not use this constructor. **/
    STSimpleEventGenerator();

    /** 
     * Standard constructor.
     * @param 
     **/
    STSimpleEventGenerator(Int_t pdg, Int_t numP, Double_t *listP, Int_t mult, Double_t x0 = 0, Double_t y0 = 0, Double_t z0 = 0, Double_t vx = 0, Double_t vy = 0, Double_t vz = 1);

    /** Destructor. **/
    virtual ~STSimpleEventGenerator();

    /** 
     * Abstract method in base class.
     * Reads event from the input file and pushes the tracks onto the stack. 
     * @param primGen  pointer to the FairPrimaryGenerator
     **/
    virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);

    /** Set primary vertex position **/
    void SetPrimaryVertex(Double_t x, Double_t y, Double_t z)
    { fV3Vertex.SetXYZ(x,y,z); };

    /** Set momentum direction vector **/
    void SetMomentumDirection(Double_t vx, Double_t vy, Double_t vz)
    { fPDirection.SetXYZ(vx,vy,vz); fPDirection = fPDirection.Unit(); };

    /** Get number of events written in EventGen file **/
    Int_t GetNEvents() { return fNEvents; };

    Bool_t SetAngleStep(Int_t pgd, UInt_t numEvt, Double_t p, Double_t theta_begin, Double_t theta_end, Double_t phi_begin, Double_t phi_end);

  void SetMCFile(TString fname,
		   Int_t neve, // number of events in a mc file
		   Int_t pdg=2212, // pdg for mc
		   Double_t pfactor=1.0); // factor for momentum

  void SetEventFile(TString fname);
  void SetRealEvent(int eventid);

  private :
    Int_t     fPDG;            ///< Particle number
    Double_t  fP;              ///< Momentum
    TVector3  fV3Vertex;       ///< Position of primary vertex
    TVector3  fPDirection;     ///< Momentum direction vector
    Int_t     fNEvents;        ///< Total number of events
    Int_t     fCurrentEvent;   ///< Current event number
    Int_t     fRealEvent;   ///< Real event number
    Int_t     fMultiplicity;   ///< Multiplicity of a momentum value

    Double_t *fPList;      ///< Momentum list
  TRandom3 *ran;
  TFile *fin;
  TFile *fin_eve;
  TNtuple *ntp;
  TTree *evetree;
  Float_t fX,fY,fZ;
  Int_t fTreeEventID;
  Double_t fVtxX,fVtxY,fVtxZ;

    std::vector<TVector3> fMomentum; 

    UInt_t     fEventMode;

    STSimpleEventGenerator(const STSimpleEventGenerator&);
    STSimpleEventGenerator& operator=(const STSimpleEventGenerator&);

    ClassDef(STSimpleEventGenerator,1);
};

#endif
