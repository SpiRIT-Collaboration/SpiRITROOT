/**
 * @brief SPiRIT Simple Event Generator
 * @author JungWoo Lee (Korea Univ.)
 */

#ifndef STSIMPLEEVENTGENERATOR
#define STSIMPLEEVENTGENERATOR

#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"
#include "FairLogger.h"

#include "TVector3.h"

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


  private :
    Int_t     fPDG;            ///< Particle number
    TVector3  fV3Vertex;       ///< Position of primary vertex
    TVector3  fPDirection;     ///< Momentum direction vector
    Int_t     fNEvents;        ///< Total number of events
    Int_t     fCurrentEvent;   ///< Current event number
    Int_t     fMultiplicity;   ///< Multiplicity of a momentum value

    Double_t *fPList;      ///< Momentum list

    std::vector<TVector3> fMomentum; 

    UInt_t     fEventMode;

    STSimpleEventGenerator(const STSimpleEventGenerator&);
    STSimpleEventGenerator& operator=(const STSimpleEventGenerator&);

    ClassDef(STSimpleEventGenerator,1);
};

#endif
