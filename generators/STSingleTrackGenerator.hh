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

	inline bool IsOpen(){ return file_.is_open(); };
	inline bool IsEnd() { return line_.find_first_not_of(' ') == std::string::npos; };

	void Next();
	void LoopOver();	
	TVector3 GetVertex();
private:
	std::ifstream file_;
	std::string line_;
};

class TrackParser
{
public:
	TrackParser(const std::string& t_filename)
	{
		LOG(INFO) << "Reading configuration file " << t_filename << FairLogger::endl;
		LOG(INFO) << std::setw(20) << "Key" << std::setw(20) << "Content" << FairLogger::endl;

		std::ifstream file(t_filename.c_str());
		if(!file.is_open())
			LOG(FATAL) << "Cannot read generator file " << t_filename << FairLogger::endl;

		std::string line, key, content;
		while(std::getline(file, line))
		{
			// erase everything after # char
			auto pos = line.find("#");
			if(pos != std::string::npos)
				line.erase(line.begin() + pos, line.end());

			const auto first_char = line.find_first_not_of(" \t\r\n");
			if(first_char == std::string::npos) // skip if the line is empty
				continue;


			std::stringstream ss(line);
			// first element is the key, second one is content
			ss >> key >> std::ws;
			std::getline(ss, content);
			
			// check and see if key exist
			if( keys2lines_.find(key) != keys2lines_.end() )
				LOG(ERROR) << "Key value " << key << " is duplicated. Only the newest value will be loaded\n";

			keys2lines_[key] = content;
			
			LOG(INFO) << std::setw(20) << key << std::setw(20) << content << FairLogger::endl;
		}
	};

	bool AllKeysExist(const std::vector<std::string> t_list_of_keys)
	{
		for(const auto& key : t_list_of_keys)
			if(keys2lines_.find(key) == keys2lines_.end())
				return false;
		return true;
	};

	TVector3 GetVector3(const std::string& t_key)
	{
		std::stringstream ss(keys2lines_.at(t_key));
		double x, y, z;
		if(!(ss >> x >> y >> z))
			LOG(FATAL) << t_key << " cannot be read as TVector as its content does not contain 3 values" << FairLogger::endl;

		return TVector3(x, y, z);
	};

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

    std::string  fVertexFile;

    STSingleTrackGenerator(const STSingleTrackGenerator&);
    STSingleTrackGenerator& operator=(const STSingleTrackGenerator&);

    VertexReader fVertexReader;   

    ClassDef(STSingleTrackGenerator,1);
};

#endif

