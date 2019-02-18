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
#include "TCutG.h"

#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <istream>
#include <iostream>
#include <stdio.h>
#include <iomanip>

class VertexReader
{
public:
	VertexReader(){};

	void OpenFile(const std::string& t_filename);

	inline bool IsOpen(){ return vectors_.size() > 0; };
	inline bool IsEnd() { return it_ == vectors_.end(); };

	void Next() {++it_;};
	void LoopOver() {it_ = vectors_.begin();};	
	int GetNumEvent() { int num = vectors_.size(); LOG(INFO) << " Number of events: " << num << FairLogger::endl; return num;};
	TVector3 GetVertex() { return *it_;};
private:
	std::vector<TVector3> vectors_;
	std::vector<TVector3>::iterator it_;
};

class TrackParser
{
public:
	TrackParser(const std::string& t_filename);

	bool AllKeysExist(const std::vector<std::string>& t_list_of_keys);
	bool KeyExist(const std::string& t_key);
	TVector3 GetVector3(const std::string& t_key);
	std::pair<double, double> GetBound(const std::string& t_key);

	template<class T>
	std::vector<T> GetList(const std::string& t_key)
	{
		std::vector<T> var;
		std::stringstream ss(keys2lines_.at(t_key));
		T temp;
		while((ss >> temp)) var.push_back(temp);
		return var;
	}

	template<class T>
	T Get(const std::string& t_key)
	{
		T var;
		std::stringstream ss(keys2lines_.at(t_key));
		ss >> var;
		return var;		
	};
private:
	std::map<std::string, std::string> keys2lines_;
};


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
    void ReadConfig(const std::string&);
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

    // set random momentum flag
    void SetRandomMomentum(Bool_t f, Double_t m0, Double_t m1)
    { fRandomMomentum = f; fMomentumRange[0] = m0; fMomentumRange[1] = m1; }

    // set random direction flag
    void SetUniformRandomDirection(Bool_t f) { fUniRandomDirection = f; }
    void SetSphericalRandomDirection(Bool_t f) { fSpheRandomDirection = f; }
    // use with random direction. change the range of angles
    void SetThetaPhiLimit(Double_t t0, Double_t t1, Double_t p0, Double_t p1)
    { fThetaRange[0] = t0; fThetaRange[1] = t1; fPhiRange[0] = p0; fPhiRange[1] = p1; }
    void SetThetaLimit(Double_t t0, Double_t t1) { fThetaRange[0] = t0; fThetaRange[1] = t1; }
    void SetPhiLimit(Double_t p0, Double_t p1)   { fPhiRange[0] = p0; fPhiRange[1] = p1; }
    void SetGausMomentum(Double_t mean, Double_t sd) {fGausMomentum = kTRUE; fGausMomentumMean = mean; fGausMomentumSD = sd;}
    void SetGausPhi(Double_t mean, Double_t sd) {fGausPhi = kTRUE; fGausPhiMean = mean; fGausPhiSD = sd;}
    void SetGausTheta(Double_t mean, Double_t sd) {fGausTheta = kTRUE; fGausThetaMean = mean; fGausThetaSD = sd;}
    void SetPhaseSpaceCut(const std::string& filename);

    // set parameters as cocktail beam run, argument is E/A setting
    void SetCocktailEvent(Double_t);
    // set brho. all inputs will have the same brho by this.
    void SetBrho(Double_t b) { fBrho = b; }

    // load vertex location from file (it set)
    void SetVertexFile(const std::string& filename) { fVertexReader.OpenFile(filename); };

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

    Bool_t fRandomMomentum;
    Double_t fMomentumRange[2];

    Bool_t   fUniRandomDirection; // uniform distribution within -180<phi<180 deg, 0<theta<90 deg.
    Bool_t   fSpheRandomDirection; // spherical distribution within -180<phi<180 deg, 0<theta<90 deg.
    Bool_t   fUniTheta;
    Bool_t   fUniPhi;
    Double_t fThetaRange[2];
    Double_t fPhiRange[2];
    Bool_t   fGausMomentum;
    Double_t fGausMomentumMean;
    Double_t fGausMomentumSD;
    Bool_t   fGausTheta;
    Double_t fGausThetaMean;
    Double_t fGausThetaSD;
    Bool_t   fGausPhi;
    Double_t fGausPhiMean;
    Double_t fGausPhiSD;

    Bool_t   fIsCocktail;
    Double_t fBrho;

    Bool_t   fIsDiscreteTheta;
    Bool_t   fIsDiscretePhi;
    Int_t    fNStepTheta;
    Int_t    fNStepPhi;


    TCutG    *fPhaseSpaceCut = nullptr;

    Int_t GetQ(Int_t);
    Int_t GetA(Int_t);

    std::string  fVertexFile;

    STSingleTrackGenerator(const STSingleTrackGenerator&);
    STSingleTrackGenerator& operator=(const STSingleTrackGenerator&);

    VertexReader fVertexReader;   

    ClassDef(STSingleTrackGenerator,1);
};

#endif

