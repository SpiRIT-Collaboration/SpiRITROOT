/**
 * @brief SPiRIT Single track Generator
 * @author Masanori Kaneko
 */
#ifndef STSINGLETRACKGENERATOR
#define STSINGLETRACKGENERATOR

#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"
#include "FairLogger.h"

#include "TVector3.h"

#include <vector>


/***
 *
 * In default, this class produces one proton track in one event with (0., 0., 500.) [MeV] momentum
 * You can set particle type, direction, momentum, brho, ,,,
 *
 * momentum value set is treated as proton's brho
 *
 * ***/


class STSingleTrackGenerator : public FairGenerator
{
  public:
    STSingleTrackGenerator();
    virtual ~STSingleTrackGenerator();

    virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);
    Int_t GetNEvents(){ return fNEvents; }
    void SetNEvents(Int_t e){ fNEvents = e; }

    // set particle which you want to use by array of pdg code.
    void SetParticleList(Int_t*);
    void SetParticleList(std::vector<Int_t> v) { fPdgList = v; }

    // set primary vertex. (target center is used as default. see constructor)
    void SetPrimaryVertex(TVector3 v) { fPrimaryVertex = v; }

    // set momentum for particle.
    void SetMomentum(TVector3 m)   { fMomentum = m; }
    void SetMomentum(Double_t mag) { fMomentum.SetMag(mag); }
    void SetDirection(TVector3 d)  { fMomentum.SetMagThetaPhi(fMomentum.Mag(),d.Theta(),d.Phi()); }
    void SetThetaPhi(Double_t t, Double_t p)  { fMomentum.SetMagThetaPhi(fMomentum.Mag(),t,p); }

    // set random direction flag
    void SetRandomDirection(Bool_t f) { fRandomDirection = f; }
    // use with random direction. change the range of angles
    void SetThetaPhiLimit(Double_t t0, Double_t t1, Double_t p0, Double_t p1)
    { fThetaRange[0] = t0; fThetaRange[1] = t1; fPhiRange[0] = p0; fPhiRange[1] = p1; }
    void SetThetaLimit(Double_t t0, Double_t t1) { fThetaRange[0] = t0; fThetaRange[1] = t1; }
    void SetPhiLimit(Double_t p0, Double_t p1)   { fPhiRange[0] = p0; fPhiRange[1] = p1; }

    // set parameters as cocktail beam run, argument is E/A setting
    void SetCocktailEvent(Double_t);
    // set brho. all inputs will have the same brho by this.
    void SetBrho(Double_t b) { fBrho = b; }

    // set discrete angle distribution. set nDivision and angle range
    void SetDiscreteTheta(Int_t s, Double_t t0, Double_t t1)
    { fIsDiscreteTheta = kTRUE; fNStepTheta = s; SetThetaLimit(t0,t1); }
    void SetDiscretePhi(Int_t s, Double_t p0, Double_t p1)
    { fIsDiscretePhi = kTRUE; fNStepPhi = s; SetPhiLimit(p0,p1); }

    void RegisterHeavyIon();

  private:
    Int_t    fNEvents;
    std::vector<Int_t> fPdgList;   // particle pdg list
    TVector3 fPrimaryVertex;
    TVector3 fMomentum;

    Bool_t   fRandomDirection; // uniform distribution within -180<phi<180 deg, 0<theta<90 deg.
    Double_t fThetaRange[2];
    Double_t fPhiRange[2];

    Bool_t   fIsCocktail;
    Double_t fBrho;

    Bool_t   fIsDiscreteTheta;
    Bool_t   fIsDiscretePhi;
    Int_t    fNStepTheta;
    Int_t    fNStepPhi;


    Int_t GetQ(Int_t);
    Int_t GetA(Int_t);


    STSingleTrackGenerator(const STSingleTrackGenerator&);
    STSingleTrackGenerator& operator=(const STSingleTrackGenerator&);

    ClassDef(STSingleTrackGenerator,1);
};

#endif

