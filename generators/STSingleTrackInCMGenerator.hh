/**
 * @brief SPiRIT Single track Generator
 * @author Masanori Kaneko
 */
#ifndef STSINGLETRACKINMCGENERATOR
#define STSINGLETRACKINMCGENERATOR

#include "FairGenerator.h"
#include "FairPrimaryGenerator.h"
#include "STSingleTrackGenerator.hh"
#include "FairLogger.h"
#include "STBeamEnergy.hh"
#include "STBDCProjection.hh"
#include "STFairMCEventHeader.hh"

#include "TVector3.h"
#include "TCutG.h"
#include "TFile.h"
#include "TTree.h"

#include <map>
#include <vector>
#include <sstream>
#include <string>
#include <istream>
#include <iostream>
#include <stdio.h>
#include <iomanip>

class BeamAngleLoader
{
public: 
  BeamAngleLoader(){};
  ~BeamAngleLoader(){};

  virtual bool GetEntry(int i) = 0;
  double GetProjA(){ return fProjA; };
  double GetProjB(){ return fProjB; };
protected:
  Double_t fProjA = -999, fProjB = -999;
};

class BDCInfoLoader : public BeamAngleLoader
{
public:
  BDCInfoLoader(int runNo, double vertexZ);
  ~BDCInfoLoader(){};

  virtual bool GetEntry(int i);
  double GetProjA(){ return fProjA; };
  double GetProjB(){ return fProjB; };
protected:
  TString fBeamFilename = "";
  TFile fBeamFile;
  TTree *fBeamTree = nullptr;
  Double_t fZ, fAoQ, fBeta37;
  TTree *fBDCTree;
  Double_t fBDC1x, fBDC1y, fBDC2x, fBDC2y, fBDCax, fBDCby;
  STBeamEnergy *fBeamEnergy = nullptr;
  STBDCProjection *fBDCProjection = nullptr;
  Double_t fVertexZ;
};

class MCBDCInfoLoader : public BeamAngleLoader
{
public:
  MCBDCInfoLoader(const TString& fileName);
  virtual bool GetEntry(int i);
private: 
  TFile fBeamFile;
  TTree *fTree;
  STFairMCEventHeader *fHeader;
  std::map<int, int> fEventIDToEntry;
};

class NullInfoLoader : public BeamAngleLoader
{
public:
  NullInfoLoader() { fProjA = fProjB = 0; };
  virtual bool GetEntry(int i) {return true; };
};

/***
 *
 * In default, this class produces one proton track in one event with (0., 0., 500.) [MeV] momentum
 * You can set particle type, direction, momentum, brho, ,,,
 *
 * momentum value set is treated as proton's brho
 *
 * ***/


class STSingleTrackInCMGenerator : public FairGenerator
{
  public:
    STSingleTrackInCMGenerator();
    virtual ~STSingleTrackInCMGenerator();

    virtual Bool_t ReadEvent(FairPrimaryGenerator* primGen);
    Int_t GetNEvents(){ return fNEvents; }
    void ReadConfig(const std::string&);
    void SetNEvents(Int_t e){ fNEvents = e; }

    // set particle which you want to use by array of pdg code.
    void SetParticleList(Int_t*);
    void SetParticleList(std::vector<Int_t> v) { fPdgList = v; }

    // set primary vertex. (target center is used as default. see constructor)
    void SetPrimaryVertex(TVector3 v) { fPrimaryVertex = v; }

    // load vertex location from file (it set)
    void SetVertexFile(const std::string& filename) { fVertexReader.OpenFile(filename); };
    void SetVertexBegin(int t_begin) { fVertexReader.BeginAt(t_begin); fParticleReader.BeginAt(t_begin);}

    // use this if the source is a mc digi file
    void SetMCBeamFile(TString filename) { fMCBeamFile = filename; }
    // use this if you don't want to perform any beam rotation
    void SetNoBeamRotation() { fNoBeamRotation = true; }
    void RegisterHeavyIon();

  private:
    Int_t    fNEvents;
    Double_t fPtMin, fPtMax;
    Double_t fCMzMin, fCMzMax;
    std::vector<Int_t> fPdgList;   // particle pdg list
    std::vector<std::pair<double, double>> fPhiRanges;
    TVector3 fPrimaryVertex;
    TVector3 fMomentum;

    Int_t GetQ(Int_t);
    Int_t GetA(Int_t);
    Int_t fBeamMass;
    Int_t fTargetMass;
    Double_t fBeamEnergyPerN;

    std::string  fVertexFile;
    TString fMCBeamFile;

    STSingleTrackInCMGenerator(const STSingleTrackInCMGenerator&);
    STSingleTrackInCMGenerator& operator=(const STSingleTrackInCMGenerator&);

    VertexReader fVertexReader;   
    VertexReader fParticleReader;
    std::unique_ptr<BeamAngleLoader> fBDCInfo;
    const double      fNucleonMass = 0.9315;
    Bool_t fNoBeamRotation = false;

    ClassDef(STSingleTrackInCMGenerator,1);
};

#endif

